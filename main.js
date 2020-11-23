const { createSVGWindow } = require("svgdom");
const window = createSVGWindow();
const SVG = require("svg.js")(window);
const document = window.document;
const csv = require("csv-load-sync");
const fs = require("fs");
const util = require("util");
const exec = util.promisify(require("child_process").exec);

// SVG files use 96 points per inch
function inch(n) {
  return n * 96;
}

// All layout is done in 1000x space to allow for whole numbers. Adjust back to inches.
function ainch(n) {
  return (n / 1000) * 96;
}


function drawBoard(canvas, x, y, height, width, margin, rough_padding, label) {
  const group = canvas.group();
  let background = "white";
  if (label.indexOf("Stock") !== -1) {
    background = "#eee";
    rough_padding = 0;
    margin = 2 * margin;
  }

  // label = label.replace(/ /g, '\n');
  if (width > 3) {
    label = label + `\n${height} x ${width}"`;
  } else {
    label = label + ` - ${height} x ${width}"`;
  }

  //group.rect(inch(width - (2 * margin)), inch(height - (2 * margin))).fill(background).stroke({ width: 1, color: '#000'}).move(inch(x + margin), inch(y + margin));

  // group.rect(inch(width), inch(height)).fill(background).stroke({ width: 1, color: '#000'}).move(inch(x), inch(y));

  // if (rough_padding) {
  // group.rect(inch(width - (2 * margin)), inch(height - (2 * margin))).fill(background).stroke({ width: 1, color: '#000', dasharray: '5,5' }).move(inch(x + margin), inch(y + margin));
  // }

  // TEMP: Disable because it's far too small to see
  // group.rect(inch(width - (2 * margin) - (2 * rough_padding)), inch(height - (2 * margin) - (2 * rough_padding))).fill(background).stroke({ width: 1, color: '#000', dasharray: '15,15' }).move(inch(x + margin + rough_padding), inch(y + margin + rough_padding));
  centeredText(group, label, 15, width / 2 + x, height / 2 + y, true);
}

function drawStock(canvas, x, y, height, width, margin) {
  const group = canvas.group();
  let background = "#eee";

  group
    .rect(inch(width), inch(height))
    .fill(background)
    .stroke({ width: 1, color: "#ff0" })
    .move(inch(x), inch(y));
  // group.rect(inch(width - (2 * margin)), inch(height - (2 * margin))).fill(background).stroke({ width: 1, color: '#000', dasharray: '15,15' }).move(inch(x + margin), inch(y + margin));
}

function drawOffcut(canvas, x, y, height, width) {
  const group = canvas.group();
  let background = "#f00";

  group
    .rect(inch(width), inch(height))
    .fill("#f00")
    .stroke({ width: 0, color: "#000" })
    .move(inch(x), inch(y));
}

function ddo(canvas, x, y, height, width) {
  const group = canvas.group();
  let background = "#f00";

  group
    .rect(inch(width), inch(height))
    .fill("#fff")
    .stroke({ width: 0, color: "#000" })
    .move(inch(x), inch(y));
}

function drawOffcut2(canvas, box, parent, ox, oy) {
  const group = canvas.group();
  ox = ox * 1000;
  oy = oy * 1000;

  // console.log(`Comparing ${box.X} to ${parent.X}`);
  // TODO(add stock offsets)
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

  // console.log(`Comparing ${box.Y} to ${parent.Y}`);
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

  // console.log(`Comparing ${box.Y + box.HEIGHT} to ${parent.Y + box.HEIGHT}`);
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

  // console.log(`Comparing ${box.X + box.WIDTH} to ${parent.X + parent.WIDTH}`);
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

  // const group = canvas.group()
  // let background = '#f00';
  // group.rect(inch(width), inch(height)).fill('#fff').stroke({ width: 1, color: '#000'}).move(inch(x), inch(y));
}

function drawBoard2(canvas, box, parent, ox, oy) {
  const group = canvas.group();
  ox = ox * 1000;
  oy = oy * 1000;

  console.log(`Comparing ${box.X} to ${parent.X}`);
  // TODO(add stock offsets)
  if (box.X !== parent.X) {
    group
      .line(
        ainch(box.X + ox),
        ainch(box.Y + oy),
        ainch(box.X + ox),
        ainch(box.Y + box.HEIGHT + oy)
      )
      .stroke({ width: 1, color: "#000" });
  }

  console.log(`Comparing ${box.Y} to ${parent.Y}`);
  if (box.Y !== parent.Y) {
    group
      .line(
        ainch(box.X + ox),
        ainch(box.Y + oy),
        ainch(box.X + box.WIDTH + ox),
        ainch(box.Y + oy)
      )
      .stroke({ width: 1, color: "#000" });
  }

  console.log(`Comparing ${box.Y + box.HEIGHT} to ${parent.Y + box.HEIGHT}`);
  if (box.Y + box.HEIGHT !== parent.Y + parent.HEIGHT) {
    group
      .line(
        ainch(box.X + ox),
        ainch(box.Y + box.HEIGHT + oy),
        ainch(box.X + box.WIDTH + ox),
        ainch(box.Y + box.HEIGHT + oy)
      )
      .stroke({ width: 1, color: "#000" });
  }

  console.log(`Comparing ${box.X + box.WIDTH} to ${parent.X + parent.WIDTH}`);
  if (box.X + box.WIDTH !== parent.X + parent.WIDTH) {
    group
      .line(
        ainch(box.X + box.WIDTH + ox),
        ainch(box.Y + oy),
        ainch(box.X + box.WIDTH + ox),
        ainch(box.Y + box.HEIGHT + oy)
      )
      .stroke({ width: 1, color: "#000" });
  }

  // const group = canvas.group()
  // let background = '#f00';
  // group.rect(inch(width), inch(height)).fill('#fff').stroke({ width: 1, color: '#000'}).move(inch(x), inch(y));
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

function isReferenced(boxes, node_id) {
  for (const box of boxes) {
    if (box.PARENT === node_id) return true;
  }
  return false;
}

(async function run() {
  const KERF = 0.125 / 2;
  const ROUGH_PADDING = 0.125;

  // Amount of the board lost when jointing and cutting down
  const BOARD_WASTE = 0.125;

  const data = csv("boards.csv");
  const stock = csv("stock.csv");
  const boxes = [];

  const items = [];

  let currentX = 0;
  let stockMargin = 3;
  for (const row of stock) {
    row.top_margin = parseFloat(row.top_margin);
    row.bottom_margin = parseFloat(row.bottom_margin);
    row.w = parseFloat(row.width) - 2 * BOARD_WASTE;
    row.h =
      parseFloat(row.height) -
      2 * BOARD_WASTE -
      row.top_margin -
      row.bottom_margin;
    row.x = currentX + stockMargin;
    currentX += stockMargin + row.w;
    row.y = stockMargin;
  }

  for (const row of data) {
    row.quantity = parseInt(row.quantity, 10);
    row.w = parseFloat(row.width) + 2 * KERF + 2 * ROUGH_PADDING;

    // Multiple copies of one board will always be layed out together.
    row.h =
      (parseFloat(row.height) + 2 * KERF + 2 * ROUGH_PADDING) * row.quantity;
    items.push(row);
  }

  // Write stock and items files
  const stockData = [["ID", "WIDTH", "HEIGHT"]];
  for (let i = 0; i < stock.length; ++i) {
    stockData.push([
      i,
      Math.floor(stock[i].w * 1000),
      Math.floor(stock[i].h * 1000),
    ]);
  }
  fs.writeFileSync("/Users/jimmy/TEST_bins.csv", stockData.join("\n"));

  const itemsData = [["ID", "WIDTH", "HEIGHT", "COPIES"]];
  for (let i = 0; i < items.length; ++i) {
    // We've already doubled the height of duplicated boards (to place them near eachother).
    // So we reduce quantity to one here.
    itemsData.push(
      [i, Math.floor(items[i].w * 1000), Math.floor(items[i].h * 1000), 1].join(
        ","
      )
    );
  }
  fs.writeFileSync("/Users/jimmy/TEST_items.csv", itemsData.join("\n"));

  // TODO(jimmy): Move the executable into this directory
  const {
    stdout,
    stderr,
  } = await exec(
    '/Users/jimmy/src/packingsolver/bazel-bin/packingsolver/main -v -p RG -i /Users/jimmy/TEST -c /Users/jimmy/TEST_solution.csv -o /Users/jimmy/TEST_output.json -t 4 -q "RG -p 3NHO" -a "IMBA* -c 4"  -q "RG -p 3NHO" -a "IMBA* -c 5"',
    { shell: true }
  );

  // create svg.js instance
  const canvas = SVG(document.documentElement).size(inch(50), inch(200));

  // Draw our stock
  for (const board of stock) {
    const h = board.h + board.top_margin + board.bottom_margin; // + (2 * BOARD_WASTE);
    const w = board.w; // + (2 * BOARD_WASTE);

    drawStock(canvas, board.x, board.y, h, w, KERF);
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

  const solution = csv("/Users/jimmy/TEST_solution.csv");
  // Fix up types
  for (const box of solution) {
    box.X = parseInt(box.X, 10);
    box.Y = parseInt(box.Y, 10);
    box.HEIGHT = parseInt(box.HEIGHT, 10);
    box.WIDTH = parseInt(box.WIDTH, 10);
  }

  for (const box of solution) {
    const originalBoard = items[box.TYPE];
    const parentStock = stock[box.PLATE_ID];
    const x = box.X / 1000 + parentStock.x; // + BOARD_WASTE;
    const y = box.Y / 1000 + parentStock.y; // + BOARD_WASTE //  + parentStock.top_margin;

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

    if (originalBoard) {
      originalBoard.placed = true;

      // drawBoard(canvas, x, y, box.HEIGHT / 1000, box.WIDTH / 1000, MARGIN, ROUGH_PADDING, originalBoard.title);

      if (originalBoard.quantity === 1) {
        // board(canvas, box.x, box.y, box.h, box.w, MARGIN, ROUGH_PADDING, box.title);
        drawBoard(
          canvas,
          x,
          y,
          box.HEIGHT / 1000,
          box.WIDTH / 1000,
          KERF,
          ROUGH_PADDING,
          originalBoard.title
        );
        // drawBoard(canvas, x, y, box.HEIGHT / 1000, box.WIDTH / 1000, KERF, ROUGH_PADDING, box.NODE_ID);
      } else {
        const group = canvas.group();
        const partialHeight = box.HEIGHT / 1000 / originalBoard.quantity;
        for (var i = 0; i < originalBoard.quantity; ++i) {
          const title = `${originalBoard.title} #${i}`;
          drawBoard(
            group,
            x,
            y + i * partialHeight,
            partialHeight,
            box.WIDTH / 1000,
            KERF,
            ROUGH_PADDING,
            title
          );
        }

        // Draw a line at each point
        for (var i = 0; i < originalBoard.quantity; ++i) {
          console.log(
            "**** DRAWING LINE AT",
            y + i * partialHeight,
            x,
            x + box.WIDTH / 1000
          );
          group
            .line(
              inch(x),
              inch(y + i * partialHeight),
              inch(x + box.WIDTH / 1000),
              inch(y + i * partialHeight)
            )
            .stroke({ width: 1, color: "#000" });
          // const title = `${originalBoard.title} #${i}`;
          //drawBoard(group, x, y + (i * partialHeight), partialHeight, box.WIDTH / 1000, KERF, ROUGH_PADDING, title);
        }
      }
      // drawBoard2(canvas, box, parent, parentStock.x, parentStock.y);
      // drawOffcut(canvas, x, y, box.HEIGHT / 1000, box.WIDTH / 1000);
    } else {
      //drawOffcut(canvas, x, y, box.HEIGHT / 1000, box.WIDTH / 1000);
      //if (box.NODE_ID === '2') {
      drawOffcut2(canvas, box, parent, parentStock.x, parentStock.y);
      //ddo(canvas, x, y, box.HEIGHT / 1000, box.WIDTH / 1000);
      //}
      if (!isReferenced(solution, box.NODE_ID)) {
        drawOffcut(canvas, x, y, box.HEIGHT / 1000, box.WIDTH / 1000);
        // drawOffcut(canvas, x, y, box.HEIGHT / 1000, box.WIDTH / 1000, KERF, ROUGH_PADDING, 'X' + box.NODE_ID);
      }
      //drawBoard(canvas, x, y, box.HEIGHT / 1000, box.WIDTH / 1000, KERF, ROUGH_PADDING, 'X' + box.NODE_ID);
    }
  }

  for (let i = 0; i < items.length; ++i) {
    if (!items[i].placed) {
      console.log(`Failed to place board ${i}: ${items[i].title}`);
    }
  }

  fs.writeFileSync("out.svg", canvas.svg());
})();
