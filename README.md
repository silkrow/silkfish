<h1 style="display: inline;">
  Silkfish
  <img src="images/logo64.gif" alt="Silkfish Logo" style="vertical-align: middle;"/>
</h1>

## 1. Compile the Engine
***silkfish*** engine is written in c++, so you do need a ```g++``` compiler to compile it. 

To compile, run the following command at the current directory,

    g++ -std=c++17 -O3 -march=native -o silkfish main.cpp

This should give you an executable ```silkfish``` at the current directory, being the engine itself &#x270C;

## 2. To Play with GUI (Python3 needed)
You can interact with ***silkfish*** engine with a commandline GUI written in Python3. However, you do need to install some libraries first, run the following command at the current directory **if you use pip3 to install Python3 libraries**,

    pip3 install -r gui_requirements.txt

If you haven't compiled the engine, you need to compile it following the "Compile the Engine" section!!

Now, with the compiled engine (```silkfish``` executable) and the installed Python3 libraries, you can run the following command at the current directory to enjoy interacting with the engine! &#x1F37A;

    python3 gui.py

This is how the command line GUI looks like:

<img src="images/gui.png" alt="GUI" style="width: 300px;"/>

## 3. Use the Engine
**After compilation**, you can use the engine without a GUI, there're two possible ways.

The first way to use the engine is to enjoy seeing a game played by itself,

    ./silkfish <-flag> demo <depth>

    e.g.
    ./silkfish demo 6

The second way to use the engine is to give it a position in the format of FEN, and receive an output from it.

    ./silkfish <-flag> <depth> {fen_string} 

    e.g.
    ./silkfish -m 7 4k3/8/6K1/8/3Q4/8/8/8 w - - 0 1

### 3.1 Flags
No matter how many flags you pass, pass it with a single ```-```, for example, to use flag ```a``` and ```m```, you can either do ```-am``` or ```-ma```.

1. ```m```: Mute the output of engine (expect the final results).

## 4. Testing

### 4.1 Eigenmann Rapid Engine Test
This is a test containing 111 positions, each position is expected to be solved in 15 seconds. Run it with 

    python3 test_eret.py

Below is a result table of ```silkfish``` on the ERET test.

| Commit Hash   | Position Passed (out of 111)   | Pass Rate   | Date |
|------------|------------|------------|------------|
| 93f488c | 13 (without time limit)| 11.71% | 2024.9.6 |
| aa7f91f | 11 | 9.91% | 2024.9.7 |
