#!/bin/sh

RESULT=/tmp/dlt-daemon/$2
LOG=$RESULT/$(basename $1).log

FN_usage()
{
    echo "Usage: ${CMD_NAME} gtest_binary output_path"
    exit 1
}

FN_copy_gcno()
{
    array=`find . -type f -name "*.gcda" -printf '%P\n' | sed "s/gcda/gcno/"`

    # Copy gcno
    echo "  Copy gcno"
    for a in $array; do
        GCNO=/$a
        DIR=$(dirname $a)

        cp $GCNO $DIR
    done
}

FN_copy_gcda_gcno()
{
    # Copy gcda and gcno to common folder
    echo "  Copy gcda and gcno to common folder"
    find . -name "*.gcda" | xargs -I{} cp {} .
    find . -name "*.gcno" | xargs -I{} cp {} .
}

FN_generate_result()
{
    # Generate result
    echo "  Generate result"
    lcov -d . -c -o coverage.info > /dev/null
    lcov -r coverage.info */gtest-1.7.0/* */x86_64-linux-gnu/* */c++/* */tests/* */include/* -o coverageFiltered.info > /dev/null
    genhtml -o lcovHtml --num-spaces 4 -s --legend coverageFiltered.info > /dev/null
    rm coverage.info coverageFiltered.info
}

#############################################################
## main
#############################################################

CMD_NAME=`basename $0`

while getopts :h OPT
do
    case $OPT in
        h) FN_usage
           ;;
        \?) FN_usage
            ;;
    esac
done

shift $((OPTIND - 1))

if [ $# -ne 2 ]
then
    FN_usage
fi

echo "########################################"
echo "Run gtest: $1"

if [ ! -f $1 ]; then
    echo "gtest does not exist. Run on valid folder."
    exit 1
fi

echo "  Output the result to $RESULT"
mkdir -p $RESULT
export GCOV_PREFIX=$RESULT

# Run gtest
./$1 > ${LOG}

pushd $RESULT > /dev/null

FN_copy_gcno
FN_copy_gcda_gcno
FN_generate_result

echo "  Result can be found in $RESULT/lcovHtml/index.html"
echo "########################################"

popd > /dev/null

