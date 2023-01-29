#!/bin/sh
cmake . "-DCI_DOC_GENERATE=ON"
cd docs/html
echo "" > .nojekyll
