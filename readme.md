## How to compile and run

ctrl + shift + b
from MSYS UCRT terminal run ./src/atoms
 
### OR for faster compilation:

#### do this one time for each new shell
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"

#### then to compile with UCRT C++ version (it includes OpenGL)
g++ -g -DGLEW_STATIC src/atom.cpp -o atom.exe -static -lglew32 -lglfw3 -lopengl32 -lgdi32
#### Run compiled program
./atom.exe