
struct Fruit {
	double mass = 0.0; // in grams
	// This class is the base class for all fruits e.g. Orange, Apple
	Fruit() {}
	void SetMass(double grams) {mass = grams;}
	double GetMass() const {return mass;}
	virtual ~Fruit() {}
	virtual const char* GetName() const {return "Fruit";}
	void Consume(double grams);
};

struct Orange : Fruit {
	Orange() {}
	const char* GetName() const override;
};

struct Apple : Fruit {
	Apple() {}
	const char* GetName() const {return "Apple";}
};

void Fruit::Consume(double grams) {
	if (grams > mass)
		grams = mass;
	mass -= grams;
}

const char* Orange::GetName() const {
	return "Orange";
}

#define TEST(x) if (o.GetMass() == 0) return 1;

int main(int argc, const char *argv[])
{
	Orange o;
	o.SetMass(120); // grams
	o.Consume(20);
	TEST(o);
	
	return 0;
}
