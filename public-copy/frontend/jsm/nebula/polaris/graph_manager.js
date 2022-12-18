import { Graph } from './sub_graph.js';
import { Debug } from '../utils/debug.js.js.js.js.js.js';
// let _container;
// let _numGraphs;
// let _cellWidth = 400;
// let _cellHeight = 300;
// let _graphs = [];
// let tickerList = [];
// let parameterList = [];
// let pairList = [];
// let _api = new WSApi();
// let _id = -1;


let _header = {
    ticker: "undefined",
    parameter: "undefined",
    graph: "undefined"
}

// class Carousel {
//     constructor(frameCt) {
//         this.frames = [];
//         this.frameCt = frameCt;
//     }
// };

class GraphManager {
    constructor(id) {
        this._container = document.getElementById(id);
        this._carousel = false;
        this.numGraphs;
        this.cellWidth = 400;
        this.cellHeight = 300;
        this.graphs = [];
        this.frameMap = [];
        this.tickerList = [];
        this.parameterList = [];
        this.pairList = [];
        this._api = new WSApi();
        this._id = -1;
        this.count= -1;
        this.frameCt = 4;
        this.head = 0;
        this.range = [0, 1, 2, 3];

        // this._container.style.gridTemplateColumns = `repeat(${this.frameCt}, fr)`;
        this._container.style.gridTemplateColumns = `repeat(${this.frameCt}, 420px)`;
        // this._container.children.forEach((child) => {
        //     console.log(child);
        // });
        this.init();
    }

    add(tick, param, direction="vertical") {
        let available = true;
        for (let i = 0; i < this.graphs.length; i++) {
            if (this.graphs[i].ticker == tick && this.graphs[i].parameter == param) {
                console.error("Graph already exists");
                available = false;
                break;
            } 
        }

        let primaryVar = true;
        if (this.tickerList.includes(tick)) {
            primaryVar = false;
            console.log("%c REGISTERING AS SECONDARY", "background: pink;");
        } else {
            console.log("%c REGISTERING AS PRIMARY", "background: crimson;");
        }

        if (available) {
            console.log("Adding graph to visualizer");
            if (primaryVar != true) {
                this.count = this.getTickerFrame(tick);
            } else {
                this.count++;
            }

            console.log(`%c COUNT IS: ${this.count}`, "color: teal;");
            this.frameMap.push([tick, this.count]);
            console.log("FRAME MAP LENGTH: " + this.frameMap.length);
            let g = new Graph(this.cellWidth, this.cellHeight, `graph-${tick}`, tick, param, this.count, primaryVar);
            // let o = { ticker: tick, type: "chart", parameter: param };
            // let self = this;
            // this._api.sendCommand("stock", o).then(function(data){
            //     g.seed("historical", data.result.notification.data);
            //     let o = {
            //         ticker: tick,
            //         parameter: param,
            //         graph: g,
            //         frame: self.count
            //     };
            //     console.log("% Adding graph with frame: ", "color; cyan;");
            //     console.log(self.count);
                self.graphs.push(g);
            //     self.tickerList.push(tick);
            //     self.parameterList.push(param);
            //     self.pairList.push([tick, param]);
            //     g.init();
            // });
            // console.log("Length of graphs is: "+this.graphs.length);
        }
    }

    getTickerFrame(ticker) {
        console.log("BUG MAY BE FUCKING HERE!!!!\n\n\n\n\n\n");
        console.log("start bug search");
        let result = -1;
        Debug.color("orange", "ticker elements");
        console.log("Searching for: " + ticker);
        this.frameMap.forEach((el) => {
            console.log(el);
            console.log(el[0]);
            if (el[0] == ticker) {
                console.log("EUREKA MF");
                result = el[1];
            }
        });
        console.log("%c End bug search", "color: red;");
        console.log("RESULT IS : " + result);
        return result;
    }

    vertical(parameter) {
    }

    horizontal(ticker_, parameter) {
    }

    init() {
        this.loop();
    }

    next() {
        console.log("graph ct currently at: "+this.count);
        if (this.frameCt >= (this.count + 1)) {
            console.log("Next and Previous disabled");
        } else {
            this.head++;
            let i = 0;
            this.range.forEach((idx) => {
                idx++;
                if (idx >= this.count + 1) {
                    idx -= (this.count + 1);
                }
                this.range[i] = idx;
                i++;
            });
        }
        console.log("THE RANGES ARE NOW AS FOLLOWS: ");
        console.log(this.range);
    }

    prev() {
        console.log("graph ct currently at: "+this.count);
        if (this.frameCt >= (this.count + 1)) {
            console.log("Next and Previous disabled");
        } else {
            this.head--;
            let i = 0;
            this.range.forEach((idx) => {
                idx--;
                if (idx < 0) {
                    idx += (this.count + 1);
                }
                this.range[i] = idx;
                i++;
            });
        }
        console.log("THE RANGES ARE NOW AS FOLLOWS: ");
        console.log(this.range);
    }

    in() {
        console.log("Zooming in");
    }

    out() {
        console.log("Zooming out");
    }

    sleep(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }


    exists(type, value) {
        switch (type) {
            case "ticker":
                this.tickerList.forEach((el) => {
                    if (el == value) { return true; }
                });
                break;
            case "parameter":
                this.parameterList.forEach((el) => {
                    if (el == value) { return true; }
                });
                break;
            case "pair":
                this.pairList.forEach((el) => {
                    if (el == value) { return true; }
                });
                break;
            default:
                console.error("Invalid type on graph manager exists(type, value)");
                break;
        }
        return false;
    } 

    // wizard of oz?
    extendz(parameter = ['volume']) {
        console.log("Extending vertically");
        let self = this;
        console.log("All the graphs are:");
        console.log(this.graphs);
        parameter.forEach((p) => {
            console.log("SELF TICKER LIST IS: " + self.tickerList.length);
            for (let i = 0; i < self.tickerList.length; i++) {                
                this.add(self.tickerList[i], p, "useless");
            }
        });
    }

    resize(width, height) {
        console.log("Graphs size: "+this.graphs.length);
        for (let i = 0; i < this.graphs.length; i++) {
            this.graphs[i].graph.resize(width, height);
        }
    }

    clear() {

    }

    loop() {
        console.log("Graphs length is: " + this.graphs.length);
        for (let i = 0; i < this.graphs.length; i++) {
            let g = this.graphs[i].graph;
            if (!g.running) { g.start(g.frame); }
        }
        requestAnimationFrame(this.loop.bind(this));
    }

    get carousel() { return this._carousel; }
    set carousel(val) { this._carousel = val; }

} export { GraphManager }