#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
export LD_LIBRARY_PATH=$DIR/../lib
$DIR/ComBombGui
