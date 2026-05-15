#!/bin/bash

cd ../src && compiledb  -S -f make && mv compile_commands.json ..

cd ../src && /c/Users/bbrown/AppData/Local/Python/pythoncore-3.14-64/Scripts/compiledb  -S -f make && mv compile_commands.json ..