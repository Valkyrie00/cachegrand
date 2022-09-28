/**
 * Copyright (C) 2018-2022 Vito Castellano
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 **/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "matcher.h"
#include "builder.h"
#include "support.h"

#include "analyzer.h"

test_t* analyzer_analyze(
        char *file_path) {
    char *body = support_read_file(file_path);
    test_t *test = builder_new_test_p();

    analyzer_recursive_match(
            body,
            START_PADDING,
            test,
            NULL);

    return test;
}

int analyzer_recursive_match(
        const char *body,
        int padding,
        test_t *current_test,
        section_t *father_section) {
    // Match SECTIONS
    matcher_t *sections = matcher_get_sections(body, padding);
    if (sections->n_matches <= 0) {
        free(sections);
        return 0;
    }

    for (int i = 0; i < sections->n_matches; ++i) {
        section_t *current_section = builder_new_section_p();
        // Match SECTION name
        char *section_name = matcher_get_section_name(sections->matches[i]);
        if (NULL == section_name) continue;
        current_section->name = section_name;

        // Match REQUIRE section
        matcher_t *section_requires = matcher_get_requires_section(sections->matches[i], padding);
        if (section_requires->n_matches > 0) {
            for (int j = 0; j < section_requires->n_matches; ++j) {
                // Match COMMAND
                char *command = matcher_get_require_command(section_requires->matches[j]);
                if (NULL == command) continue;
                builder_section_append_command(
                        current_section, command);
            }
        }
        free(section_requires);

        if (NULL != father_section) {
            builder_section_append_subsection(
                    father_section,
                    current_section);
        } else {
            builder_test_append_section(
                    current_test,
                    current_section);
        }

        analyzer_recursive_match(
                sections->matches[i],
                padding*2,
                current_test,
                current_section);
    }

    free(sections);
}

void analyzer_recursive_print(
        section_t **sections,
        size_t n_sections) {
    for (int i = 0; i < n_sections; ++i) {
        section_t *section = sections[i];
        printf("SECTION: %s\n", section->name);
        if (section->n_commands > 0) {
            for (int j = 0; j < section->n_commands; ++j) {
                printf("\tCOMMAND: %s\n", section->commands[j]);
            }
        }

        if (section->n_subsections > 0) {
            analyzer_recursive_print(
                    section->subsections,
                    section->n_subsections);
        }
    }

    printf("\n");
}