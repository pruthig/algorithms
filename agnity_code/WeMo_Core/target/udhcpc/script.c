/* script.c
 *
 * Functions to call the DHCP client notification scripts 
 *
 * Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include "options.h"
#include "dhcpd.h"
#include "dhcpc.h"
#include "packet.h"
#include "options.h"
#include "debug.h"

#include "nvram.h"

/* get a rough idea of how long an option will be (rounding up...) */
static int max_option_length[] = {
	[OPTION_IP] =		sizeof("255.255.255.255 "),
	[OPTION_IP_PAIR] =	sizeof("255.255.255.255 ") * 2,
	[OPTION_STRING] =	1,
	[OPTION_BOOLEAN] =	sizeof("yes "),
	[OPTION_U8] =		sizeof("255 "),
	[OPTION_U16] =		sizeof("65535 "),
	[OPTION_S16] =		sizeof("-32768 "),
	[OPTION_U32] =		sizeof("4294967295 "),
	[OPTION_S32] =		sizeof("-2147483684 "),
};


static int upper_length(int length, struct dhcp_option *option)
{
	return max_option_length[option->flags & TYPE_MASK] *
	       (length / option_lengths[option->flags & TYPE_MASK]);
}


static int sprintip(char *dest, char *pre, unsigned char *ip) {
	return sprintf(dest, "%s%d.%d.%d.%d ", pre, ip[0], ip[1], ip[2], ip[3]);
}


/* Fill dest with the text of option 'option'. */
static void fill_options(char *dest, unsigned char *option, struct dhcp_option *type_p)
{
	int type, optlen;
	u_int16_t val_u16;
	int16_t val_s16;
	u_int32_t val_u32;
	int32_t val_s32;
	int len = option[OPT_LEN - 2];

	dest += sprintf(dest, "%s=", type_p->name);

	type = type_p->flags & TYPE_MASK;
	optlen = option_lengths[type];
	for(;;) {
		switch (type) {
		case OPTION_IP_PAIR:
			dest += sprintip(dest, "", option);
			*(dest++) = '/';
			option += 4;
			optlen = 4;
		case OPTION_IP:	/* Works regardless of host byte order. */
			dest += sprintip(dest, "", option);
 			break;
		case OPTION_BOOLEAN:
			dest += sprintf(dest, *option ? "yes " : "no ");
			break;
		case OPTION_U8:
			dest += sprintf(dest, "%u ", *option);
			break;
		case OPTION_U16:
			memcpy(&val_u16, option, 2);
			dest += sprintf(dest, "%u ", ntohs(val_u16));
			break;
		case OPTION_S16:
			memcpy(&val_s16, option, 2);
			dest += sprintf(dest, "%d ", ntohs(val_s16));
			break;
		case OPTION_U32:
			memcpy(&val_u32, option, 4);
			dest += sprintf(dest, "%lu ", (unsigned long) ntohl(val_u32));
			break;
		case OPTION_S32:
			memcpy(&val_s32, option, 4);
			dest += sprintf(dest, "%ld ", (long) ntohl(val_s32));
			break;
		case OPTION_STRING:
			memcpy(dest, option, len);
			dest[len] = '\0';
			return;	 /* Short circuit this case */
		}
		option += optlen;
		len -= optlen;
		if (len <= 0) break;
	}
}


static char *find_env(const char *prefix, char *defaultstr)
{
	extern char **environ;
	char **ptr;
	const int len = strlen(prefix);

	for (ptr = environ; *ptr != NULL; ptr++) {
		if (strncmp(prefix, *ptr, len) == 0)
			return *ptr;
	}
	return defaultstr;
}


/* put all the paramaters into an environment */
static char **fill_envp(struct dhcpMessage *packet)
{
	int num_options = 0;
	int i, j;
	char **envp;
	unsigned char *temp;
	char over = 0;

	if (packet == NULL)
		num_options = 0;
	else {
		for (i = 0; options[i].code; i++)
			if (get_option(packet, options[i].code))
				num_options++;
		if (packet->siaddr) num_options++;
		if ((temp = get_option(packet, DHCP_OPTION_OVER)))
			over = *temp;
		if (!(over & FILE_FIELD) && packet->file[0]) num_options++;
		if (!(over & SNAME_FIELD) && packet->sname[0]) num_options++;		
	}
	
	envp = xmalloc((num_options + 5) * sizeof(char *));
	envp[0] = xmalloc(sizeof("interface=") + strlen(client_config.interface));
	sprintf(envp[0], "interface=%s", client_config.interface);
	envp[1] = find_env("PATH", "PATH=/bin:/usr/bin:/sbin:/usr/sbin");
	envp[2] = find_env("HOME", "HOME=/");

	if (packet == NULL) {
		envp[3] = NULL;
		return envp;
	}

	envp[3] = xmalloc(sizeof("ip=255.255.255.255"));
	sprintip(envp[3], "ip=", (unsigned char *) &packet->yiaddr);
	for (i = 0, j = 4; options[i].code; i++) {
		if ((temp = get_option(packet, options[i].code))) {
			envp[j] = xmalloc(upper_length(temp[OPT_LEN - 2], &options[i]) + strlen(options[i].name) + 2);
			fill_options(envp[j], temp, &options[i]);
			j++;
		}
	}
	if (packet->siaddr) {
		envp[j] = xmalloc(sizeof("siaddr=255.255.255.255"));
		sprintip(envp[j++], "siaddr=", (unsigned char *) &packet->siaddr);
	}
	if (!(over & FILE_FIELD) && packet->file[0]) {
		/* watch out for invalid packets */
		packet->file[sizeof(packet->file) - 1] = '\0';
		envp[j] = xmalloc(sizeof("boot_file=") + strlen(packet->file));
		sprintf(envp[j++], "boot_file=%s", packet->file);
	}
	if (!(over & SNAME_FIELD) && packet->sname[0]) {
		/* watch out for invalid packets */
		packet->sname[sizeof(packet->sname) - 1] = '\0';
		envp[j] = xmalloc(sizeof("sname=") + strlen(packet->sname));
		sprintf(envp[j++], "sname=%s", packet->sname);
	}	
	envp[j] = NULL;
	return envp;
}


/* Call a script with a par file and env vars */
void run_script(struct dhcpMessage *packet, const char *name)
{
	int pid;
	char **envp;

	if (client_config.script == NULL)
		return;

	/* call script */
	pid = fork();
	if (pid) {
		waitpid(pid, NULL, 0);
		return;
	} else if (pid == 0) {
		envp = fill_envp(packet);
		
		/* close fd's? */
		
		/* exec script */
		DEBUG(LOG_INFO, "execle'ing %s", client_config.script);
		execle(client_config.script, client_config.script,
		       name, NULL, envp);
		LOG(LOG_ERR, "script %s failed: %s",
		    client_config.script, strerror(errno));
		exit(1);
	}			
}

/* return 1 = settings changed , 0 = no change */
int detect_ACK_options(struct dhcpMessage *packet)
{
	int	re_bound = 0;
	int i;
	unsigned char *temp;
	char over = 0;

	char option_string[64] = {0};
	char cmd[64];

	sprintip(option_string, "ip=", (unsigned char *) &packet->yiaddr);
	//sprintf(cmd, "echo \"option=%s\" > /dev/console", option_string);
	//system(cmd);
	
	re_bound = compare_nvram_settings(option_string);

	for (i = 0; options[i].code; i++) {
		if ((temp = get_option(packet, options[i].code))) {
			fill_options(option_string, temp, &options[i]);
			//sprintf(cmd, "echo \"option=%s\" > /dev/console", option_string);
			//system(cmd);
			re_bound = compare_nvram_settings(option_string);
			if(re_bound) return re_bound;
		}
	}

	return re_bound;
}

/* return 1 = settings changed , 0 = no change */
int compare_nvram_settings(char *opt_value)
{
	char *tmp;
	char cmd[64];
	
	if(strstr(opt_value, "ip="))
	{
		opt_value[strlen(opt_value)-1]='\0';
		tmp = opt_value + 3;
		if(!strcmp(nvram_safe_get("wan_ipaddr"), tmp))
		{
			//system("echo \"ip_the_same\" > /dev/console");
			return 0;
		}
		else
		{
			//sprintf(cmd, "echo \"tmp=[%s]\" > /dev/console", tmp);
			//system(cmd);
			
			//sprintf(cmd, "echo \"wan_ipaddr=[%s]\" > /dev/console", nvram_safe_get("wan_ipaddr"));
			//system(cmd);
			return 1;
		}
	}
	else if(strstr(opt_value, "subnet="))
	{
		opt_value[strlen(opt_value)-1]='\0';
		tmp = opt_value + 7;
		if(!strcmp(nvram_safe_get("wan_netmask"), tmp))
		{
			//system("echo \"netmask_the_same\" > /dev/console");
			return 0;
		}
		else
		{
			//sprintf(cmd, "echo \"tmp=[%s]\" > /dev/console", tmp);
			//system(cmd);
			
			//sprintf(cmd, "echo \"wan_netmask=[%s]\" > /dev/console", nvram_safe_get("wan_netmask"));
			//system(cmd);
			return 1;
		}
	}
	else if(strstr(opt_value, "router="))
	{
		opt_value[strlen(opt_value)-1]='\0';
		tmp = opt_value + 7;
		if(!strcmp(nvram_safe_get("wan_gateway"), tmp))
		{
			//system("echo \"wan_gateway_the_same\" > /dev/console");
			return 0;
		}
		else
		{
			//sprintf(cmd, "echo \"tmp=[%s]\" > /dev/console", tmp);
			//system(cmd);
			
			//sprintf(cmd, "echo \"wan_gateway=[%s]\" > /dev/console", nvram_safe_get("wan_gateway"));
			//system(cmd);
			return 1;
		}
	}
	else if(strstr(opt_value, "dns="))
	{
		tmp = opt_value + 4;
		if(!strcmp(nvram_safe_get("wan_nameservs"), tmp))
		{
			//system("echo \"wan_nameservs_the_same\" > /dev/console");
			return 0;
		}
		else
		{
			//sprintf(cmd, "echo \"tmp=[%s]\" > /dev/console", tmp);
			//system(cmd);
			
			//sprintf(cmd, "echo \"wan_nameservs=[%s]\" > /dev/console", nvram_safe_get("wan_nameservs"));
			//system(cmd);
			return 1;
		}
	}
	else if(strstr(opt_value, "wins="))
	{
		opt_value[strlen(opt_value)-1]='\0';
		tmp = opt_value + 4;
		if(!strcmp(nvram_safe_get("wan_winsserv"), tmp))
		{
			//system("echo \"wan_winsserv_the_same\" > /dev/console");
			return 0;
		}
		else
		{
			//sprintf(cmd, "echo \"tmp=[%s]\" > /dev/console", tmp);
			//system(cmd);
			
			//sprintf(cmd, "echo \"wan_winsserv=[%s]\" > /dev/console", nvram_safe_get("wan_winsserv"));
			//system(cmd);
			return 1;
		}
	}
	else if(strstr(opt_value, "domain="))
	{
		tmp = opt_value + 7;
		if(!strcmp(nvram_safe_get("wan_domain"), tmp))
		{
			//system("echo \"wan_domain_the_same\" > /dev/console");
			return 0;
		}
		else
		{
			//sprintf(cmd, "echo \"tmp=[%s]\" > /dev/console", tmp);
			//system(cmd);
			
			//sprintf(cmd, "echo \"wan_domain=[%s]\" > /dev/console", nvram_safe_get("wan_domain"));
			//system(cmd);
			return 1;
		}
	}
	else if(strstr(opt_value, "lease="))
	{
		tmp = opt_value + 6;
		if(!strcmp(nvram_safe_get("wan_lease"), tmp))
		{
			//system("echo \"wan_lease_the_same\" > /dev/console");
			return 0;
		}
		else
		{
			//sprintf(cmd, "echo \"tmp=[%s]\" > /dev/console", tmp);
			//system(cmd);
			
			//sprintf(cmd, "echo \"wan_lease=[%s]\" > /dev/console", nvram_safe_get("wan_lease"));
			//system(cmd);
			return 1;
		}
	}
	return 0;
	
}
