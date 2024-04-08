#include <iostream>

int main() {
	int i, N = 5;
	for(i = 0; i < N; i++){
		switch(fork()) {
			case 0:
				std::cout << i << std::endl;
			case -1:
				std::cout << "failed" << std::endl;
			default:
				std::cout << "parent work" << std::endl;
		}
	}
	while(i--) wait(NULL);
}
