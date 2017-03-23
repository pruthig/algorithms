import java.util.ArrayList;
import java.util.List;

public class StrategyPatternWiki {

	public static void main(String[] args) {
		Customer a = new Customer(new NormalStrategy());

		// Normal billing
		a.add(1.0, 1);

		// Start Happy Hour
		a.setStrategy(new HappyHourStrategy());
		a.add(1.0, 2);

		// New Customer
		Customer b = new Customer(new HappyHourStrategy());
		b.add(0.8, 1);
		// The Customer pays
		a.printBill();

		// End Happy Hour
		b.setStrategy(new NormalStrategy());
		b.add(1.3, 2);
		b.add(2.5, 1);
		b.printBill();

	}
}

class Customer {

	private List<Double> drinks;
	private BillingStrategy strategy;

	public Customer(BillingStrategy strategy) {
		this.drinks = new ArrayList<Double>();
		this.strategy = strategy;
	}

	public void add(double price, int quantity) {
		drinks.add(strategy.getActPrice(price * quantity));
	}

	// Payment of bill
	public void printBill() {
		double sum = 0;
		for (Double i : drinks) {
			sum += i;
		}
		System.out.println("Total due: " + sum);
		drinks.clear();
	}

	// Set Strategy
	public void setStrategy(BillingStrategy strategy) {
		this.strategy = strategy;
	}

}

interface BillingStrategy {
	public double getActPrice(double rawPrice);
}

// Normal billing strategy (unchanged price)
class NormalStrategy implements BillingStrategy {

	@Override
	public double getActPrice(double rawPrice) {
		return rawPrice;
	}

}

// Strategy for Happy hour (50% discount)
class HappyHourStrategy implements BillingStrategy {

	@Override
	public double getActPrice(double rawPrice) {
		return rawPrice*0.5;
	}

}
