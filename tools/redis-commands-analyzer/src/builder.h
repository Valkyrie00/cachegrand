//
// Created by Vito Castellano on 11/09/22.
//

#ifndef ANALYZER_BUILDER_H
#define ANALYZER_BUILDER_H


//typedef char command_t;

typedef struct section section_t;
struct section {
    char *name;
    int n_subsections;
    section_t **subsections;
    int n_commands;
    char **commands;
};

typedef struct test test_t;
struct test {
    char    *name;
    int n_sections;
    section_t **sections;
};

typedef struct tests tests_t;
struct tests {
    int n_tests;
    test_t **tests;
};

//command_t* new_command_p();

section_t* new_section_p();

test_t* new_test_p();

tests_t* new_tests_p();

bool section_append_subsection(
        section_t *section,
        section_t *subsections);

bool section_append_command(
        section_t *section,
        char *command);

bool test_append_section(
        test_t *test,
        section_t *section);

void test_free_sections(
        test_t *test,
        int n_section);

bool tests_append_test(
        tests_t *tests,
        test_t *test);

#endif //ANALYZER_BUILDER_H
