#include <stdio.h>

int main() {
    int num;
    printf("Please enter a number:\n");
    if (scanf("%d", &num) != 1) {
        return 0;
    }

    if (num % 2 == 0) {
        printf("The number %d is even!\n", num);
        return 0;
    } else {
        printf("The number %d is odd!\n", num);
    }

    return 1;
}