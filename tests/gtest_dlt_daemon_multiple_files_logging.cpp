/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
 *
 * This file is part of COVESA Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.covesa.org/.
 */

/*!
 * \author
 * Oleg Tropmann <oleg.tropmann@daimler.com>
 * Daniel Weber <daniel.w.weber@daimler.com>
 *
 * \copyright Copyright © 2022 Daimler TSS GmbH. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file gtest_dlt_daemon_multiple_files_logging.ccpp
 */

#include <gtest/gtest.h>

int connectServer(void);

extern "C"
{
#include "dlt_common.h"
#include <syslog.h>
#include <dirent.h>
#include <string.h>
}

// publish prototypes
void configure(const char* path, const char* file_name, bool enable_limit, int file_size, int max_files_size);
void write_log_message();
void verify_multiple_files(const char* path, const char* file_name, int file_size, int max_files_size);
void verify_single_file(const char* path, const char* file_name);
void verify_in_one_file(const char* path, const char* file_name, const char* log1, const char* log2);
bool file_contains_strings(const char* abs_file_path, const char* str1, const char* str2);
int get_file_index(char* file_name);
int compare_int(const void* a, const void* b);

/**
 * Configure dlt logging using file size limits.
 */
TEST(t_dlt_logging_multiple_files, normal)
{
    const char* path = "/tmp";
    const char* file_name = "dlt.log";
    const int file_size = 128;
    const int max_file_size = 512;
    configure(path, file_name, true, file_size, max_file_size);
    write_log_message();
    EXPECT_NO_THROW(dlt_log_free());
    verify_multiple_files(path, file_name, file_size, max_file_size);
}

/**
 * Configure dlt logging using file size limits.
 * Though, due to an error during initialization dlt logging defaults to one file logging.
 */
TEST(t_dlt_logging_one_file_as_fallback, normal)
{
    const char* path = "/tmp";
    const char* file_name = "dltlog";
    configure(path, file_name, true, 128, 512);
    write_log_message();
    EXPECT_NO_THROW(dlt_log_free());
    verify_single_file(path, file_name);
}

/**
 * Configure dlt logging without file size limits resulting in one file logging.
 */
TEST(t_dlt_logging_one_file, normal)
{
    const char* path = "/tmp";
    const char* file_name = "dlt.log";
    configure(path, file_name, false, 128, 512);
    write_log_message();
    EXPECT_NO_THROW(dlt_log_free());
    verify_single_file(path, file_name);
}

/**
 * The dlt_daemon calls dlt_log_init multiple times. In the past, so we create
 * unnecessary two files. The reinit have to append to the first file.
 */
TEST(t_dlt_logging_multiple_files_append_reinit, normal)
{
    const char* path = "/tmp";
    const char* file_name = "dlt.log";
    const int file_size = 256;
    const int max_file_size = 512;

    const char* log1 = "ONE\n";
    const char* log2 = "TWO\n";

    configure(path, file_name, true, file_size, max_file_size);
    dlt_vlog(LOG_INFO, "%s", log1);
    EXPECT_NO_THROW(dlt_log_free());

    configure(path, file_name, true, file_size, max_file_size);
    dlt_vlog(LOG_INFO, "%s", log2);
    EXPECT_NO_THROW(dlt_log_free());
    verify_in_one_file(path, file_name, log1, log2);
}

void configure(const char *path, const char* file_name, const bool enable_limit, const int file_size, const int max_files_size)
{
    char abs_file_path[PATH_MAX];
    snprintf(abs_file_path, sizeof(abs_file_path), "%s/%s", path, file_name);
    printf("debug test: %s\n", abs_file_path);

    EXPECT_NO_THROW(dlt_log_set_filename(abs_file_path));
    EXPECT_NO_THROW(dlt_log_set_level(6));
    EXPECT_NO_THROW(dlt_log_init_multiple_logfiles_support(DLT_LOG_TO_FILE, enable_limit, file_size, max_files_size));
}

void write_log_message()
{
    for (unsigned int i = 0; i < 10; i++) {
        dlt_vlog(LOG_INFO, "%d. Unit test logging into multiple files if configured.\n", i);
    }
}

void verify_multiple_files(const char* path, const char* file_name, const int file_size, const int max_files_size)
{
    int sum_size = 0;
    int num_files = 0;
    int file_indices[100];

    char filename[PATH_MAX + 1];
    struct dirent *dp;
    struct stat status;

    char file_name_copy[NAME_MAX];
    strncpy(file_name_copy, file_name, NAME_MAX);
    char filename_base[NAME_MAX];
    EXPECT_TRUE(dlt_extract_base_name_without_ext(file_name_copy, filename_base, sizeof(filename_base)));
    const char *filename_ext = get_filename_ext(file_name);
    EXPECT_TRUE(filename_ext);

    DIR *dir = opendir(path);
    while ((dp = readdir(dir)) != NULL) {
        if (strstr(dp->d_name, filename_base) &&
            strstr(dp->d_name, filename_ext)) {

            snprintf(filename, sizeof(filename), "%s/%s", path, dp->d_name);

            if (0 == stat(filename, &status)) {
                EXPECT_LE(status.st_size, file_size);
                EXPECT_GE(status.st_size, file_size/2);
                sum_size += status.st_size;
                file_indices[num_files++] = get_file_index(filename);
            } else {
                EXPECT_TRUE(false);
            }
        }
    }

    EXPECT_LE(sum_size, max_files_size);
    EXPECT_GT(sum_size, 0);
    EXPECT_GT(num_files, 0);

    //check that file indices are successive in ascending order
    qsort(file_indices, num_files, sizeof(int), compare_int);
    int index = file_indices[0];
    for (int i=1; i<num_files; i++) {
        EXPECT_EQ(file_indices[i], ++index);
    }
}

void verify_single_file(const char* path, const char* file_name)
{
    char abs_file_path[PATH_MAX];
    snprintf(abs_file_path, sizeof(abs_file_path), "%s/%s", path, file_name);

    struct stat status;
    if (0 == stat(abs_file_path, &status)) {
        EXPECT_GT(status.st_size, 0);
    } else {
        EXPECT_TRUE(false);
    }
}

void verify_in_one_file(const char* path, const char* file_name, const char* log1, const char* log2)
{
    char abs_file_path[PATH_MAX + 1];
    struct dirent *dp;

    char file_name_copy[NAME_MAX];
    strncpy(file_name_copy, file_name, NAME_MAX);
    char filename_base[NAME_MAX];
    EXPECT_TRUE(dlt_extract_base_name_without_ext(file_name_copy, filename_base, sizeof(filename_base)));
    const char *filename_ext = get_filename_ext(file_name);
    EXPECT_TRUE(filename_ext);

    bool found = false;

    DIR *dir = opendir(path);
    while ((dp = readdir(dir)) != NULL) {
        if (strstr(dp->d_name, filename_base) &&
            strstr(dp->d_name, filename_ext)) {

            snprintf(abs_file_path, sizeof(abs_file_path), "%s/%s", path, dp->d_name);

            if (file_contains_strings(abs_file_path, log1, log2)) {
                found = true;
                break;
            }
        }
    }

    EXPECT_TRUE(found);
}

bool file_contains_strings(const char* abs_file_path, const char* str1, const char* str2)
{
    bool found = false;
    FILE *file = fopen(abs_file_path, "r");
    if (file != nullptr) {
        fseek (file , 0 , SEEK_END);
        long size = ftell (file);
        rewind (file);

        char* buffer = (char*) malloc(size);
        long read_bytes = fread(buffer, 1, size, file);

        EXPECT_EQ(size, read_bytes);

        if ((strstr(buffer, str1) != nullptr) &&
            (strstr(buffer, str2) != nullptr)) {

            found = true;
        }

        fclose(file);
        free(buffer);
    }
    return found;
}

int get_file_index(char* file_name)
{
    char *dot = strrchr(file_name, '.');
    *dot = '\0';

    //start with the first zero
    char *iterator = strchr(file_name, '0');
    do {} while (*(++iterator) == '0');
    //now iterator points to the first character after 0

    return atoi(iterator);
}

int compare_int(const void* a, const void* b)
{
    if (*((int*)a) == *((int*)b)) return 0;
    else if (*((int*)a) < *((int*)b)) return -1;
    else return 1;
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::FLAGS_gtest_break_on_failure = true;
    /*::testing::FLAGS_gtest_filter = "*.normal"; */
    /*::testing::FLAGS_gtest_repeat = 10000; */
    return RUN_ALL_TESTS();
}
