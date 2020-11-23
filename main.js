const { createSVGWindow } = require("svgdom");
const window = createSVGWindow();
const SVG = require("svg.js")(window);
const document = window.document;
const csv = require("csv-load-sync");
const fs = require("fs");
const os = require("os");
const path = require("path");
const util = require("util");
const process = require("process");
const exec = util.promisify(require("child_process").exec);
const commandLineArgs = require("command-line-args");
require("colors");

// SVG files use 96 points per inch
function inch(n) {
  return n * 96;
}

// All layout is done in 1000x space to allow for whole numbers. Adjust back to inches.
function ainch(n) {
  return (n / 1000) * 96;
}

// Label a board in the output
function labelBoard(canvas, x, y, height, width, label) {
  const group = canvas.group();

  // For narrow boards, we lay the label out one after another
  // TODO(jimmy): Check if label will fit and abbreviate if not.
  if (width > 3) {
    label = label + `\n${height} x ${width}"`;
  } else {
    label = label + ` - ${height} x ${width}"`;
  }

  centeredText(group, label, 15, width / 2 + x, height / 2 + y, true);
}

// Draw the outline of a piece of stock
function drawStock(canvas, x, y, height, width) {
  canvas
    .rect(inch(width), inch(height))
    .fill("#eee")
    .stroke({ width: 1, color: "#ff0" })
    .move(inch(x), inch(y));
}

function drawWaste(canvas, x, y, height, width) {
  canvas
    .rect(inch(width), inch(height))
    .fill("#f00")
    .stroke({ width: 0, color: "#000" })
    .move(inch(x), inch(y));
}

// Draw the cuts that need to be made to produce the output boards.
// The output from packingsolver is a tree describing the board you're left with
// after each cut.
//
// Each ndoe in the tree will share two or more edges with its parent since only
// one cut (horizontal or vertical) is made at each level in the tree.
//
// To avoid drawing over the top of each edge multiple times, we identify
// edges that are shared with the parent and avoid drawing them.
function drawCuts(canvas, box, parent, ox, oy) {
  const group = canvas.group();

  // Offsets are in inches, but board and canvas dimensions are in 1000xinch
  ox = ox * 1000;
  oy = oy * 1000;

  // Left vertical cut
  if (box.X !== parent.X) {
    group
      .line(
        ainch(box.X + ox),
        ainch(box.Y + oy),
        ainch(box.X + ox),
        ainch(box.Y + box.HEIGHT + oy)
      )
      .stroke({ width: 1, color: "#f0f" });
  }

  // Top horizontal cut
  if (box.Y !== parent.Y) {
    group
      .line(
        ainch(box.X + ox),
        ainch(box.Y + oy),
        ainch(box.X + box.WIDTH + ox),
        ainch(box.Y + oy)
      )
      .stroke({ width: 1, color: "#f0f" });
  }

  // Right vertical cut
  if (box.Y + box.HEIGHT !== parent.Y + parent.HEIGHT) {
    group
      .line(
        ainch(box.X + ox),
        ainch(box.Y + box.HEIGHT + oy),
        ainch(box.X + box.WIDTH + ox),
        ainch(box.Y + box.HEIGHT + oy)
      )
      .stroke({ width: 1, color: "#f0f" });
  }

  // Bottom horizontal cut
  if (box.X + box.WIDTH !== parent.X + parent.WIDTH) {
    group
      .line(
        ainch(box.X + box.WIDTH + ox),
        ainch(box.Y + oy),
        ainch(box.X + box.WIDTH + ox),
        ainch(box.Y + box.HEIGHT + oy)
      )
      .stroke({ width: 1, color: "#f0f" });
  }
}

function centeredText(canvas, text, size, x, y, rotate) {
  const t = canvas
    .text(text)
    .font({ fill: "#888", family: "osifont", size: size, anchor: "middle" });
  t.move(inch(x), inch(y));
  if (rotate) {
    t.rotate(90);
  }
}

function hasChildren(boxes, node_id) {
  for (const box of boxes) {
    if (box.PARENT === node_id) return true;
  }
  return false;
}

const optionDefinitions = [
  { name: "input", type: String, defaultOption: true },
  { name: "kerf", type: Boolean },
  { name: "groupMultipleBoards", type: Boolean },
  { name: "stockWaste", type: Number },
  { name: "boardWaste", type: Number },
  { name: "output", type: String },
];

(async function run() {
  const options = commandLineArgs(optionDefinitions);

  if (!options.input) {
    console.log(
      "You must provide a path to the input folder containing boards.csv and stock.csv e.g. cutlayout path/to/foo"
        .red
    );
    return;
  }

  var config = {};
  if (fs.existsSync(path.join(options.input, "config.json"))) {
    config = JSON.parse(
      fs.readFileSync(path.join(options.input, "config.json"))
    );
  }

  const KERF = (config.kerf || options.kerf || 0.125) / 2;
  const BOARD_WASTE = config.boardWaste || options.boardWaste || 0.125;
  const STOCK_WASTE = config.stockWaste || options.stockWaste || 0.125;
  const GROUP_BOARDS =
    config.groupMultipleBoards || options.groupMultipleBoards;

  // Spacing between boards, in inches
  const STOCK_SPACING = 3;
  const boards = csv(path.join(options.input, "boards.csv"));
  const stock = csv(path.join(options.input, "stock.csv"));

  // Parse stock inputs and add padding as needed.
  let currentX = 0;
  for (const row of stock) {
    row.top_margin = parseFloat(row.top_margin);
    row.bottom_margin = parseFloat(row.bottom_margin);
    row.w = parseFloat(row.width) - 2 * STOCK_WASTE;
    row.h =
      parseFloat(row.height) -
      2 * STOCK_WASTE -
      row.top_margin -
      row.bottom_margin;
    row.x = currentX + STOCK_SPACING;
    currentX += STOCK_SPACING + row.w;
    row.y = STOCK_SPACING;
  }

  // Parse required boards
  for (const row of boards) {
    row.quantity = parseInt(row.quantity, 10);
    row.w = parseFloat(row.width) + 2 * KERF + 2 * BOARD_WASTE;

    if (GROUP_BOARDS) {
      // Multiple copies of one board will always be layed out together.
      row.h =
        (parseFloat(row.height) + 2 * KERF + 2 * BOARD_WASTE) * row.quantity;
      row.adjustedQuantity = 1;
    } else {
      row.h = parseFloat(row.height) + 2 * KERF + 2 * BOARD_WASTE;
      row.adjustedQuantity = row.quantity;
    }
  }

  // Make a temporary directory to write intermediate files to
  const tempPath = fs.mkdtempSync(`${os.tmpdir()}${path.sep}`);

  // Write stock and items files in the format required for `packingsolver`
  const stockData = [["ID", "WIDTH", "HEIGHT"]];
  for (let i = 0; i < stock.length; ++i) {
    stockData.push([
      i,
      Math.floor(stock[i].w * 1000),
      Math.floor(stock[i].h * 1000),
    ]);
  }
  fs.writeFileSync(path.join(tempPath, "TEST_bins.csv"), stockData.join("\n"));

  const itemsData = [["ID", "WIDTH", "HEIGHT", "COPIES"]];
  for (let i = 0; i < boards.length; ++i) {
    itemsData.push(
      [
        boards[i].adjustedQuantity,
        Math.floor(boards[i].w * 1000),
        Math.floor(boards[i].h * 1000),
        1,
      ].join(",")
    );
  }
  fs.writeFileSync(path.join(tempPath, "TEST_items.csv"), itemsData.join("\n"));

  const packingsolverBinary = path.join(
    __dirname,
    "vendor/packingsolver/bazel-bin/packingsolver/main"
  );
  if (!fs.existsSync(packingsolverBinary)) {
    console.log(
      "You must build packingsolver with Bazel before running this program.".red
    );
    return;
  }

  const {
    stdout,
    stderr,
  } = await exec(
    `${packingsolverBinary} -v -p RG -i ${tempPath}/TEST -c ${tempPath}/TEST_solution.csv -o ${tempPath}/TEST_output.json -t 4 -q "RG -p 3NHO" -a "IMBA* -c 4"  -q "RG -p 3NHO" -a "IMBA* -c 5"`,
    { shell: true }
  );

  // create svg.js instance
  const canvas = SVG(document.documentElement).size(inch(50), inch(200));

  // Draw each piece of stock
  for (const board of stock) {
    const h = board.h + board.top_margin + board.bottom_margin;
    const w = board.w;

    drawStock(canvas, board.x, board.y, h, w);
    if (board.top_margin > 0) {
      centeredText(
        canvas,
        "" + board.top_margin + '"',
        15,
        board.x + w / 2,
        board.y + board.top_margin / 2
      );
    }
    if (board.bottom_margin > 0) {
      centeredText(
        canvas,
        "" + board.bottom_margin + '"',
        15,
        board.x + w / 2,
        board.y + h - board.bottom_margin / 2
      );
    }

    const label = `${board.title}\n${board.height}x${board.width}`;
    centeredText(canvas, label, 30, board.x + w / 2, board.y + h + 1.5);
  }

  const solution = csv(path.join(tempPath, "TEST_solution.csv"));
  for (const box of solution) {
    box.X = parseInt(box.X, 10);
    box.Y = parseInt(box.Y, 10);
    box.HEIGHT = parseInt(box.HEIGHT, 10);
    box.WIDTH = parseInt(box.WIDTH, 10);
  }

  // Render each cut and label each output board in the solution.
  for (const box of solution) {
    // Map boards in the output to the original set of input boards to retrieve their name.
    const originalBoard = boards[box.TYPE];

    // Determine which sheet of stock this board will be cut from.
    const parentStock = stock[box.PLATE_ID];

    const x = box.X / 1000 + parentStock.x;
    const y = box.Y / 1000 + parentStock.y + parentStock.top_margin;

    // Boards are output as a tree - find the immediate parent of this node. If no parent,
    // treat the stock as the parent.
    var parent;
    if (!box.PARENT) {
      parent = {
        X: 0,
        Y: 0,
        HEIGHT: parentStock.h * 1000,
        WIDTH: parentStock.w * 1000,
      };
    } else {
      parent = solution[box.PARENT];
    }

    // Does this map to a board in our input file? If so, label it.
    if (originalBoard) {
      originalBoard.placed = true;

      if (originalBoard.quantity === 1 || !GROUP_BOARDS) {
        labelBoard(
          canvas,
          x,
          y,
          box.HEIGHT / 1000,
          box.WIDTH / 1000,
          originalBoard.title
        );
      } else {
        // If we did group boards, we need to divide the output board up into N parts.
        const group = canvas.group();
        const partialHeight = box.HEIGHT / 1000 / originalBoard.quantity;
        for (let i = 0; i < originalBoard.quantity; ++i) {
          const title = `${originalBoard.title} #${i}`;
          labelBoard(
            group,
            x,
            y + i * partialHeight,
            partialHeight,
            box.WIDTH / 1000,
            title
          );
        }

        // Draw a line at each point to divide the sections of the board.
        for (var i = 0; i < originalBoard.quantity; ++i) {
          group
            .line(
              inch(x),
              inch(y + i * partialHeight),
              inch(x + box.WIDTH / 1000),
              inch(y + i * partialHeight)
            )
            .stroke({ width: 1, color: "#000" });
        }
      }
    } else {
      drawCuts(canvas, box, parent, parentStock.x, parentStock.y);

      // If this node has no children and no label, it is waste. Mark it as such.
      if (!hasChildren(solution, box.NODE_ID)) {
        drawWaste(canvas, x, y, box.HEIGHT / 1000, box.WIDTH / 1000);
      }
    }
  }

  // Verify that all boards were placed.
  for (let i = 0; i < boards.length; ++i) {
    if (!boards[i].placed) {
      console.log(`Failed to place board ${i}: ${boards[i].title}`.red);
    }
  }

  if (options.output) {
    const outputPath = path.resolve(__dirname, options.output);
    fs.writeFileSync(outputPath, canvas.svg());
  } else {
    console.log(canvas.svg());
  }
})();
