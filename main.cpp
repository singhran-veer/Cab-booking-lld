#include "CabBookingApplication.hpp"

// The composition root includes the implementation so this file can be compiled directly.
#include "src/cab_booking.cpp"

using namespace std;


int main() {
	return CabBookingApplication::run();
}