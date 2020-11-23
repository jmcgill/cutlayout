# Cutlist Layout

A simple cutlist optimizer with configurable options designed for working with rough lumber. It outputs a SVG file
which can be further edited if required.

## Installing
Begin by installing local nodejs dependencies.

```
npm install .
```

This uses the excellent Packing Solver (https://github.com/fontanf/packingsolver) to lay out cuts. This is a highly
performent 2D Guillotine Cut optimizer implemented in C++.

Before running `cutlayout` the solver must be built locally.

```
brew install bazel
cd vendor/packingsolver
bazel build -- //...
``` 

## Running
`cutlayout` expects a directory containing three files:

* stock.csv - The raw stock we're cutting up
* boards.csv - The output boards we want to produce
* config.json - Configuration

and is invoked as:

`npm run cutlist path/to/input/folder -o output.svg`

Each option in `config.json` can be overwritten with a command line flag with the same name.

### stock.csv
This file contains the raw stock to cut up. It allows information about end-defects to be included; this is just a bit
easier than trying to remember if you did or did not subtract a particular end.

All units are in inches

```
width,  height, title, top_defect, bottom_defect
9.5, 76, "Cherry #1", 0, 2.5
8.5, 75.5, "Cherry #2", 3, 0
8, 69, "Cherry #3", 0, 0
```

### boards.csv
This file contains the boards we're trying to produce as output.

```
quantity, width, height, title
2, 6.25, 23.15, "Bottom *"
2, 6.25, 12.25, "Side A"
2, 6.25, 12.25, "Side B"
```

### options.json
Configurable options. While these options can all be specified via the command line, it is recommended to store them
in options.json so that you can reproduce the exact layout if needed.

**groupMultipleBoards** If true, boards with quantity > 1 will always be layed out next to each-other. Increases
board similarity at the expense of yield.

**kerf** Saw kerf for cuts. Defaults to 1/8"

**stockWaste** Amount of stock that might be lost to jointing/planing. Defaults to 1/8"

**boardWaste** Amount of stock around each cut board that may be lost due to rough cuts. Defaults to 1/8"

```
{
    groupMultipleBoards: true,
    kerf: 0.125,
    stockWaste: 0.25,
    boardWaste: 0.25
}
``` 
