#!/bin/bash
mkdir dlt_lcov_report
lcov --capture --directory $1/src --output-file dlt_lcov_report/dlt_init_coverage.info > /dev/null
lcov --remove dlt_lcov_report/dlt_init_coverage.info -o dlt_lcov_report/dlt_final_coverage.info '/usr/*' '*/include/*' > /dev/null
rm dlt_lcov_report/dlt_init_coverage.info > /dev/null
genhtml dlt_lcov_report/dlt_final_coverage.info --output-directory dlt_lcov_report
