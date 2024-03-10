#include "../cjson/cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAIN_EXE "main.exe"
#define TESTS_JSON_FILE_NAME "student_tests.json"
#define INPUT_FILE_NAME "input.txt"
#define EXPECTED_OUTPUT_FILE_NAME "expected_output.txt"
#define ACTUAL_OUTPUT_FILE_NAME "output.txt"

void printDivider(void) {
    printf("\n\033[0;36m------------------------------------------------------------\033[0m\n");
}

void printTestsSummary(int failed_count) {
    printDivider();
    if (failed_count == 0) {
        printf("\n\033[1;32mAll Tests Passed! \033[0m\n");
    } else {
        printf(
                "\n\033[1;31m%d %s Failed!\033[0m\n\n",
                failed_count,
                failed_count == 1 ? "Test" : "Tests");
    }
}

/**
 * Function to write input and output cJSON objects to text files
 * @param input
 * @param output
 * @return 1 if written successfully
 * */
int writeInputOutputToFile(cJSON *input, cJSON *output) {
    // Open the input file for writing
    FILE *input_file = fopen(INPUT_FILE_NAME, "w");
    if (input_file == NULL) {
        printf("Error opening input file for writing.\n");
        return 0;
    }

    // Write input value to the file
    fprintf(input_file, "%s\n", cJSON_GetStringValue(input));

    // Close the input file
    fclose(input_file);

    // Open the output file for writing
    FILE *output_file = fopen(EXPECTED_OUTPUT_FILE_NAME, "w");
    if (output_file == NULL) {
        printf("Error opening output file for writing.\n");
        return 0;
    }

    // Write output value to the file without appending a newline character
    fputs(cJSON_GetStringValue(output), output_file);

    // Close the output file
    fclose(output_file);

    return 1;// Return success
}

/**
 * Function to compare expected and actual output
 * @return 1 if identical
 * */
int compareOutputs() {
    FILE *expected_output_file = fopen(EXPECTED_OUTPUT_FILE_NAME, "r");
    FILE *actual_output_file = fopen(ACTUAL_OUTPUT_FILE_NAME, "r");

    if (!expected_output_file || !actual_output_file) {
        printf("Test failed: Unable to open output files!\n");
        return 0;
    }

    char expected_line[100];
    char actual_line[100];

    // Read lines from both files and compare them
    while (fgets(expected_line, sizeof(expected_line), expected_output_file) != NULL &&
           fgets(actual_line, sizeof(actual_line), actual_output_file) != NULL) {
        if (strcmp(expected_line, actual_line) != 0) {
            return 0;
        }
    }

    // Check if files has same size
    long expected_size = ftell(expected_output_file);
    long actual_size = ftell(actual_output_file);
    if (expected_size != actual_size) {
        return 0;
    }

    // identical outputs
    return 1;
}

/**
 * Function to print expected and actual output of failed test
 * @param test_name
 * @return 1 if identical
 * */
void printFailedTest(const char *test_name) {
    // Open expected output file for reading
    FILE *expected_output_file = fopen(EXPECTED_OUTPUT_FILE_NAME, "r");
    if (expected_output_file == NULL) {
        printf("Failed to open expected output file.\n");
        return;
    }

    // Open actual output file for reading
    FILE *actual_output_file = fopen(ACTUAL_OUTPUT_FILE_NAME, "r");
    if (actual_output_file == NULL) {
        printf("Failed to open actual output file.\n");
        fclose(expected_output_file);
        return;
    }

    // Print test failed message
    printf("\n\033[1;31m%s - Failed!\033[0m\n\n", test_name);

    // Print expected output
    printf("\033[1;34mExpected Output: \033[0m\n");

    int ch;
    while ((ch = fgetc(expected_output_file)) != EOF) {
        putchar(ch);
    }
    printf("\n");

    // Print actual output
    printf("\033[1;34mActual Output: \033[0m\n");
    while ((ch = fgetc(actual_output_file)) != EOF) {
        putchar(ch);
    }
    printf("\n");

    // Close files
    fclose(expected_output_file);
    fclose(actual_output_file);
}

char* extractProjectNameFromPath(char *workdir) {
    int i = 0;
    while (*(workdir+i) != '\0') {
        if (*(workdir+i) == '\\' || *(workdir+i) == '/') {
            workdir = workdir+i+1;
        }
        i++;
    }
    return workdir;
}

/**
 * Function to run test
 * @param workdir project's workdir
 * @param test {name, input, output}
 * @return 1 if test passed
 * */
int runTest(char *workdir, cJSON *test) {
    // Parse test properties
    char *name = cJSON_GetObjectItemCaseSensitive(test, "name")->valuestring;
    cJSON *input = cJSON_GetObjectItemCaseSensitive(test, "input");
    cJSON *output = cJSON_GetObjectItemCaseSensitive(test, "output");


    // Write input output to tx files
    int isIOWritten = writeInputOutputToFile(input, output);
    if (!isIOWritten) {
        printf("Failed to write input and output to files.\n");
        return 0;
    }

    char *projectName = extractProjectNameFromPath(workdir);

    // Build the path to the main.exe file
    char *commandStr = malloc(
            strlen("cmd /c ") +
            strlen(workdir) + strlen("/cmake-build-debug") + strlen(projectName) + strlen(".exe")
            + strlen( " < ") + strlen(INPUT_FILE_NAME) + strlen(" > ")
            + strlen(ACTUAL_OUTPUT_FILE_NAME)
            + 1);
    if (!commandStr) {
        printf("Error parsing main exec file path.\n");
        return 0;
    }
    strcat(commandStr, "cmd /c ");
    strcat(commandStr, workdir);
    strcat(commandStr, "/cmake-build-debug");
    strcat(commandStr, projectName);
    strcat(commandStr, ".exe");
    strcat(commandStr, " < ");
    strcat(commandStr, INPUT_FILE_NAME);
    strcat(commandStr,  " > ");
    strcat(commandStr,  ACTUAL_OUTPUT_FILE_NAME);

    // Run executable
    system(commandStr);
    free(commandStr);

    // Test outputs
    printDivider();
    int isTestPassed = compareOutputs();
    if (isTestPassed) {
        printf("\n\033[1;32m%s - Passed! \033[0m\n", name);
    } else {
        printFailedTest(name);
    }

//    free(name);

    return isTestPassed;
}

/**
 * Function to get all tests from a JSON file
 * @return json tests array
 * */
cJSON *getAllTestsFromJson(char *workdir) {
    // Build the path to the tests file
    char *testsFilePath = malloc(strlen(workdir) + 1 + strlen(TESTS_JSON_FILE_NAME) + 1);
    if (!testsFilePath) {
        printf("Error parsing tests file path.\n");
        return NULL;
    }
    testsFilePath = strcat(testsFilePath, workdir);
    testsFilePath = strcat(testsFilePath, "/");
    testsFilePath = strcat(testsFilePath, TESTS_JSON_FILE_NAME);
    // Read the JSON file
    FILE *file = fopen(testsFilePath, "r");
    free(testsFilePath);
    if (file == NULL) {
        printf("Error opening JSON file.\n");
        return NULL;
    }

    // Get the size of the file
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory to store the file content
    char *json_data = (char *) malloc(file_size + 1);
    if (json_data == NULL) {
        printf("Memory allocation error.\n");
        fclose(file);
        return NULL;
    }

    // Read the file content into the allocated memory
    fread(json_data, 1, file_size, file);
    fclose(file);

    // Null-terminate the string
    json_data[file_size] = '\0';

    // Parse the JSON data
    cJSON *json = cJSON_Parse(json_data);
    free(json_data);// Free allocated memory regardless of the parsing result
    if (json == NULL) {
        printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        return NULL;
    }

    // Get the "tests" array
    cJSON *tests = cJSON_GetObjectItemCaseSensitive(json, "tests");
    if (tests == NULL || !cJSON_IsArray(tests)) {
        printf("Error: 'tests' array not found or not an array.\n");
        return NULL;
    }

    return tests;// Return the tests array
}

int main(int argc, char* argv[]) {
    // We expect 1 arg, the location of the workdir that has tests and exe in its build dir
    // project name dir must have the same name as the exe
    if (argc != 1) {
        return 1;
    }
    int failedCount = 0;
    char *workdir = argv[0];
    cJSON *tests = getAllTestsFromJson(workdir);

    // Run all tests
    cJSON *test;
    cJSON_ArrayForEach(test, tests) {
        int isPassed = runTest(workdir, test);
        if (!isPassed) {
            failedCount++;
        }
    }
    printTestsSummary(failedCount);

    // Free cJSON object
    cJSON_Delete(tests);

    // 0 indicates successful test
    return failedCount > 0;
}
