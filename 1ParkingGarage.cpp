/*
Problem: Parking Garage
Design a parking garage system using C++98 and object-oriented programming.

The garage accepts three vehicle types:
- Motorcycle: occupies 1 space; costs $2 per hour.
- Car: occupies 2 spaces; costs $5 per hour.
- Truck: occupies 4 spaces; costs $9 per hour.
Your system must support:
1. Parking a vehicle with a unique license plate.
2. Rejecting vehicles when there is insufficient space or the plate is already parked.
3. Removing a vehicle and calculating its fee from the number of hours parked.
4. Displaying all parked vehicles, their types, and available capacity.
OOP requirements
- Use an abstract Vehicle base class and derived vehicle classes.
- Use virtual functions for vehicle-specific behavior.
- Keep the garage’s internal data private.
- Ensure dynamically allocated objects are properly destroyed.
Assumptions
- The garage has a fixed capacity supplied at construction.
- Parking duration is a positive whole number of hours, supplied at checkout.
- Spaces are interchangeable; physical parking layout does not matter.
*/
#include <bits/stdc++.h>

using namespace std;

class Vehicle {
private:
    string plate;

    Vehicle(const Vehicle&);
    Vehicle& operator=(const Vehicle&);

public:
    explicit Vehicle(const string& licensePlate)
        : plate(licensePlate) {}

    virtual ~Vehicle() {}

    const string& getPlate() const {
        return plate;
    }

    virtual const char* getType() const = 0;
    virtual int getSpaces() const = 0;
    virtual int getHourlyRate() const = 0;

    long calculateFee(int hours) const {
        if (hours <= 0) {
            throw invalid_argument("Hours must be positive.");
        }

        return static_cast<long>(getHourlyRate()) * hours;
    }
};

class Motorcycle : public Vehicle {
public:
    explicit Motorcycle(const string& plate) : Vehicle(plate) {}

    virtual const char* getType() const { return "Motorcycle"; }
    virtual int getSpaces() const { return 1; }
    virtual int getHourlyRate() const { return 2; }
};

class Car : public Vehicle {
public:
    explicit Car(const string& plate) : Vehicle(plate) {}

    virtual const char* getType() const { return "Car"; }
    virtual int getSpaces() const { return 2; }
    virtual int getHourlyRate() const { return 5; }
};

class Truck : public Vehicle {
public:
    explicit Truck(const string& plate) : Vehicle(plate) {}

    virtual const char* getType() const { return "Truck"; }
    virtual int getSpaces() const { return 4; }
    virtual int getHourlyRate() const { return 9; }
};

class ParkingGarage {
private:
    int capacity;
    int usedSpaces;
    vector<Vehicle*> vehicles;

    // Prevent copying a garage that owns raw pointers.
    ParkingGarage(const ParkingGarage&);
    ParkingGarage& operator=(const ParkingGarage&);

public:
    explicit ParkingGarage(int totalSpaces)
        : capacity(totalSpaces), usedSpaces(0) {
        if (totalSpaces <= 0) {
            throw invalid_argument("Capacity must be positive.");
        }
    }

    ~ParkingGarage() {
        for (size_t i = 0; i < vehicles.size(); ++i) {
            delete vehicles[i];
        }
    }

    int getAvailableSpaces() const {
        return capacity - usedSpaces;
    }

    // Takes ownership, even if parking fails.
    bool park(Vehicle* vehicle) {
        auto_ptr<Vehicle> owner(vehicle);

        if (vehicle == 0) {
            return false;
        }

        for (size_t i = 0; i < vehicles.size(); ++i) {
            if (vehicles[i]->getPlate() == vehicle->getPlate()) {
                cout << "Rejected: plate already parked.\n";
                return false;
            }
        }

        const int spaces = vehicle->getSpaces();

        if (spaces <= 0 || spaces > getAvailableSpaces()) {
            cout << "Rejected: insufficient space or invalid vehicle.\n";
            return false;
        }

        // owner also protects against a failed allocation here.
        vehicles.push_back(vehicle);
        owner.release();
        usedSpaces += spaces;

        cout << "Parked: " << vehicle->getPlate() << '\n';
        return true;
    }

    bool checkout(const string& plate, int hours) {
        if (hours <= 0) {
            cout << "Checkout failed: hours must be positive.\n";
            return false;
        }

        for (vector<Vehicle*>::iterator it = vehicles.begin();
             it != vehicles.end(); ++it) {
            Vehicle* vehicle = *it;

            if (vehicle->getPlate() == plate) {
                const long fee = vehicle->calculateFee(hours);

                cout << "Checked out: " << plate
                     << " | Fee: $" << fee << '\n';

                usedSpaces -= vehicle->getSpaces();
                vehicles.erase(it);
                delete vehicle;
                return true;
            }
        }

        cout << "Checkout failed: plate not found.\n";
        return false;
    }

    void display() const {
        cout << "\n--- Parking Garage ---\n";

        if (vehicles.empty()) {
            cout << "No vehicles parked.\n";
        }

        for (size_t i = 0; i < vehicles.size(); ++i) {
            cout << vehicles[i]->getPlate()
                 << " | " << vehicles[i]->getType()
                 << " | Spaces: " << vehicles[i]->getSpaces()
                 << '\n';
        }

        cout << "Available spaces: " << getAvailableSpaces()
             << '/' << capacity << "\n\n";
    }
};

int main() {
    ParkingGarage garage(8);

    garage.park(new Motorcycle("M-101"));
    garage.park(new Car("C-202"));
    garage.park(new Truck("T-303"));

    garage.display();

    garage.park(new Car("C-202")); // Duplicate plate.
    garage.park(new Car("C-404")); // Insufficient space.
    garage.checkout("C-202", 3);   // Fee: $15.
    garage.park(new Car("C-404")); // Now succeeds.

    garage.display();

    return 0;
}
