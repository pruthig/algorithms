#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=Cygwin_4.x-Windows
CND_DLIB_EXT=dll
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/Main.o \
	${OBJECTDIR}/card_game.o \
	${OBJECTDIR}/counting_inversions.o \
	${OBJECTDIR}/dijkstra.o \
	${OBJECTDIR}/karger_min_cut.o \
	${OBJECTDIR}/linked_list.o \
	${OBJECTDIR}/merge_sort.o \
	${OBJECTDIR}/operator_overloading.o \
	${OBJECTDIR}/order_statistics.o \
	${OBJECTDIR}/prim_mst.o \
	${OBJECTDIR}/quicksort_median_of_3_pivot.o \
	${OBJECTDIR}/quicksort_randomized_pivot.o \
	${OBJECTDIR}/sum_of_vector_elements.o \
	${OBJECTDIR}/tree.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/netbeansproject.exe

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/netbeansproject.exe: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/netbeansproject ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/Main.o: Main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Main.o Main.cpp

${OBJECTDIR}/card_game.o: card_game.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/card_game.o card_game.cpp

${OBJECTDIR}/counting_inversions.o: counting_inversions.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/counting_inversions.o counting_inversions.cpp

${OBJECTDIR}/dijkstra.o: dijkstra.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/dijkstra.o dijkstra.cpp

${OBJECTDIR}/karger_min_cut.o: karger_min_cut.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/karger_min_cut.o karger_min_cut.cpp

${OBJECTDIR}/linked_list.o: linked_list.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/linked_list.o linked_list.cpp

${OBJECTDIR}/merge_sort.o: merge_sort.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/merge_sort.o merge_sort.cpp

${OBJECTDIR}/operator_overloading.o: operator_overloading.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/operator_overloading.o operator_overloading.cpp

${OBJECTDIR}/order_statistics.o: order_statistics.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/order_statistics.o order_statistics.cpp

${OBJECTDIR}/prim_mst.o: prim_mst.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/prim_mst.o prim_mst.cpp

${OBJECTDIR}/quicksort_median_of_3_pivot.o: quicksort_median_of_3_pivot.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/quicksort_median_of_3_pivot.o quicksort_median_of_3_pivot.cpp

${OBJECTDIR}/quicksort_randomized_pivot.o: quicksort_randomized_pivot.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/quicksort_randomized_pivot.o quicksort_randomized_pivot.cpp

${OBJECTDIR}/sum_of_vector_elements.o: sum_of_vector_elements.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/sum_of_vector_elements.o sum_of_vector_elements.cpp

${OBJECTDIR}/tree.o: tree.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tree.o tree.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/netbeansproject.exe

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
