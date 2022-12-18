// let this.linesX, this.linesY, this.xAxis, this.yAxis;
// let this.xBins, this.yBins;
import * as Util from './util.js';
import * as Algo from './algorithms.js';

let debug = false;



let graphCt = 0;
let frameCt = 4;

class Graph {
    constructor(width, height, wrapper_id, ticker, parameter, frame, primary, frameCtt = 4) {
        this.width = width;
        this.height = height;
        this.size = 25;

        this.cursor = {
            x: 0,
            y: 0,
            active: false,
        };
        // console.log("sdfsdfsdfsdf\n\n\n\n\n\n\n\n");
        // console.log(graphCt);
        let vis = document.getElementById("graph-manager");
        let ct = vis.getAttribute("data-count");
        vis.setAttribute("data-count", ct++);

        this._wrapper = undefined;
        this.initWrapper(wrapper_id);
        
        let id = Util.generateUUID();
        this._frame = frame;
        this.id = id; 

        // create canvas
        this.canvas = document.createElement("canvas");
        this.canvas.setAttribute("width", this.width);
        this.canvas.setAttribute("height", this.height);
        this.canvas.style.borderRadius = '10px';
        this.canvas.style.margin = '10px';
        this.canvas.style.boxShadow = 'var(--shadow)';
        // this.canvas.style.display = 'none';
        this.canvas.setAttribute("id", id);

        // maybe use in draw
        if (primary) {
            console.log("Primary graph being rendered");
            if (graphCt < frameCt) {
                document.getElementById(`grid-frame-${graphCt}`).append(this.canvas);
            } else {
                this._wrapper.append(this.canvas);
            }
        } else {
            console.log("Secondary graph being rendered");
            console.log("Frame is: "+frame);
            document.getElementById(`grid-frame-${frame}`).append(this.canvas);
        }

        // document.getElementById("graph-manager").append(wrapper);
        
        this.canvas.style.backgroundColor = '#0e0e0e';
        this.ctx = this.canvas.getContext('2d');
        this.linesX = Math.floor(this.height/this.size);
        this.linesY = Math.floor(this.width/this.size);
        this.labels_ = [];
        this.points = [];
        this._running = false;
        this.lineSets = [];
        this.volume = [];
        this.transactions = [];
        this.pointsNorm = [];
        this.zoomY = 0;
        this.zoomX = 0;
        this.xAxis = this.linesX - 1;
        this.yAxis = 1;
        this.xEnd = 0; 
        this.xBegin = 0;
        this.highlighting = true;
        this.hovering = true;
        this.collidables = [];
        this.fillGraph = [];
        this.ticker = "undefined";
        this.lineOffset = 0;
        this.active = false;
        this.updateReady = false;
        this.xStart = { number: 1, suffix: '\u03a0' };
        this.yStart = { number: 1, suffix: '' };
        this.xBins = 0;
        this.yBins = 0;
        this.rawDataX = [];
        this.rawDataY = [];
        this.pointMap = [];
        this.xLabel = "none";
        this.yLabel = "none";
        this.weightA = 75;
        this.weightB = 25;
        this.xLabel = "Time (ET)";
        this.yLabel = "Unknown";
        this.state = {
            pressed: false,
            drag: false,
            lastPos: [0, 0]
        };
        if (parameter == "rsi") {
            this.lineColor = "purple";
        } else {
            this.lineColor = "green";
        }
        this.ticker = ticker;
        this._label = ticker;
        this._subLabel = "N/A";
        this._parameter = parameter;
        this.chartType = "line";

        if (parameter == "volume") {
            this.type = "bar";
        } else if (parameter == "pie") {
            this.type = "pie";
        } else {
            this.type = "line";
        }
        console.warn("Initializing graph!");
        graphCt++;
    }

    addLine(points) {
        this.lineSets.push(points);
    }

    get wrapper() { return this._wrapper.getAttribute("id"); }
    set wrapper(val) { this._wrapper.setAttribute("id", val); } 

    computeDims() {
        if (debug) console.log("computing dims");
        let ogXRange = this.width - this.size;
        let ogYRange = this.height - this.size;
        if (debug) console.log(`Pixel ranges are <${ogXRange},${ogYRange}>`);
        let yRange = (1 -this.zoomY) * ogYRange;
        if (this.zoomY != 0) {
            this.lineOffset = ogYRange * (this.zoomY / 2);
        } else {
            this.lineOffset = 0;
        }

        if (this.zoomX == 0) {
            this.xEnd = this.points.length;
        } else {
            this.xEnd = this.points.length - this.zoomX;
        }

        let xRange = ogXRange;
        this.xBins = xRange / (this.points.length - this.zoomX * 2);
        this.yBins = yRange / (this.points.length - this.zoomX * 2);

        this.pointMap = new Map();
        // for (let i = 0; i < this.points.length; i++) {
        for (let i = this.xBegin; i < this.xEnd; i++) {
            let yVal = -(this.pointsNorm[i] * this.yBins * this.points.length + this.lineOffset);
            this.pointMap.set(i * this.xBins, (yVal + this.height) - this.lineOffset);
        }

        if (debug) console.log("# of bins is: <"+this.xBins+","+this.yBins+">");
    }

    drawTooltip(x, y) {
        if (this.active == true) {
            this.ctx.lineWidth = 5;
            this.ctx.beginPath();
            this.ctx.moveTo(x, y);
            this.ctx.lineTo(x + 50, y);
            this.ctx.lineTo(x + 50, y - 50);
            this.ctx.lineTo(x, y - 50);
            // this.ctx.closePath();
            // this.ctx.fillStyle = 'red';
            let points = this.intersect();
            // points = [1, 2];

            this.ctx.beginPath();
            this.ctx.fillStyle = '#22222299';
            this.ctx.fillRect(this.cursor.x, this.cursor.y, 150, 50);
            this.ctx.strokeStyle = "#ffffff88";
            this.ctx.lineWidth = "1";
            this.ctx.strokeRect(this.cursor.x, this.cursor.y, 150, 50);
            this.ctx.stroke();
            this.ctx.fillStyle = '#ffffff';
            this.ctx.font = '12px Arial';
            // this.ctx.fillText("Hello", this.cursor.x + 10, this.cursor.y + 20);
            this.ctx.fillText(`${this.xLabel}: ${points[0]}`, this.cursor.x + 10, this.cursor.y + 20);
            this.ctx.fillText(`${this.yLabel}: ${points[1]}`, this.cursor.x + 10, this.cursor.y + 40);
            this.ctx.fill();
        }
    }

    drawFill() {
        this.ctx.lineWidth = 2;
        this.ctx.beginPath();
        this.ctx.moveTo(this.fillGraph[0].x, this.fillGraph[0].y);
        for (let i = 0; i < this.fillGraph.length; i++) { this.ctx.lineTo(this.fillGraph[i].x, this.fillGraph[i].y); }
        this.ctx.closePath();
        var grd = this.ctx.createLinearGradient(0, 0, this.canvas.width, this.canvas.height);
        grd.addColorStop(0, '#00000000');
        grd.addColorStop(1, '#00000000');   
        this.ctx.fillStyle = grd;
        this.ctx.fill();
    }

    intersect() {
        let result = 0;
        let xLock = 0;
        console.log("XBIN LENGTH IS: " + this.xBins);
        console.log("cursor.x: " + this.cursor.x);
        let numX = ((this.width - this.lineOffset) / this.xBins) + this.lineOffset;
        for(let i = 0; i < Math.ceil(numX); i++) {
            if (i * this.xBins - (this.cursor.x) <= this.xBins) {
                result = this.xBins * i;
                xLock = i;
                // break;
            }
        }
        this.drawLineIntersect(result);
        return [this.rawDataX[xLock], this.rawDataY[xLock]];
    }

    drawLineIntersect(xVal) {
        let yVal = this.pointMap.get(xVal);
        this.ctx.beginPath();
        console.log("DRAWING CURSOR CIRCLE AT: <" + xVal + ", " + yVal + ">");
        this.ctx.arc(xVal, yVal, 5, 0, 2 * Math.PI);
        this.ctx.fillStyle = "magenta";
        this.ctx.fill();
    }

    drawCandle(points) {
        this.ctx.save();
        this.ctx.translate(this.yAxis * this.size, this.xAxis * this.size);
        this.ctx.beginPath();
        this.ctx.moveTo(0, 0);
        for (let i = this.xBegin; i < this.xEnd; i++) {
            let yVal = -(this.pointsNorm[i] * this.yBins * this.points.length + this.lineOffset);
            // this.ctx.lineTo((i - this.xBegin) * this.xBins, yVal);
            // this.fillGraph.push({x: (i - this.xBegin) * this.xBins, y: yVal});

            this.ctx.fillRect(i * this.xBins, yVal, this.xBins, yVal);
        }
        this.ctx.strokeStyle = "#00ff00";
        this.ctx.stroke();
        // this.drawFill();
        this.ctx.restore();
    }

    drawBar(points) {
        this.ctx.save();
        this.ctx.translate(this.yAxis * this.size, this.xAxis * this.size);
        this.ctx.beginPath();
        this.ctx.moveTo(0, 0);
        for (let i = this.xBegin; i < this.xEnd; i++) {
            let lastY = 0;
            if (i > this.xBegin) {
                lastY = -(points[i - 1] * this.yBins * this.points.length + this.lineOffset);
            }
            let yVal = -(points[i] * this.yBins * this.points.length + this.lineOffset);

            if (yVal <= lastY) {
                this.ctx.fillStyle = "#00ff00";
            } else {
                this.ctx.fillStyle = "#ff0000";
            }
            // this.ctx.lineTo((i - this.xBegin) * this.xBins, yVal);
            // this.fillGraph.push({x: (i - this.xBegin) * this.xBins, y: yVal});

            this.ctx.fillRect(i * this.xBins, 0, this.xBins, yVal);
        }
        this.ctx.strokeStyle = "#00ff00";
        this.ctx.stroke();
        // this.drawFill();
        this.ctx.restore();
    }

    drawPie() {
        let radA = (this.weightA / (this.weightA + this.weightB)) * 2 * Math.PI;
        let radB = (this.weightB / (this.weightA + this.weightB)) * 2 * Math.PI;
        this.ctx.beginPath();
        this.ctx.fillStyle = "#0fff00";
        this.ctx.strokeStyle = "#ffffff";
        this.ctx.moveTo(this.width / 2, this.height / 2);
        this.ctx.arc(this.width / 2, this.height / 2, (this.width / 2) - 100, -Math.PI / 2, radA - Math.PI / 2);
        this.ctx.lineTo(this.width / 2, this.height / 2);
        this.ctx.stroke();
        this.ctx.fill();
        this.ctx.beginPath();
        this.ctx.fillStyle = "#ff0000";
        this.ctx.strokeStyle = "#ffffff";
        this.ctx.moveTo(this.width / 2, this.height / 2);
        this.ctx.arc(this.width / 2, this.height / 2, (this.width / 2) - 100, radA - (Math.PI / 2), -Math.PI / 2);
        this.ctx.lineTo(this.width / 2, this.height / 2);
        this.ctx.stroke();
        this.ctx.fill();
    }

    drawDoubleBar() {

    }

    initWrapper(wrapper_id) {
        let vis = document.getElementById("graph-manager");
        this._wrapper = document.getElementById(wrapper_id);
        if (this._wrapper == undefined) {
            let el = document.createElement("div");
            el.setAttribute("id", wrapper_id);
            vis.append(el);
            this._wrapper = document.getElementById(wrapper_id);
        }
        this._wrapper.style.height = '100%';
        this._wrapper.style.display = 'block';
    }

    drawLine(points) {
        this.ctx.lineWidth = 2;
        this.ctx.save();
        this.ctx.translate(this.yAxis * this.size, this.xAxis * this.size);
        this.ctx.beginPath();
        this.ctx.moveTo(0, 0);
        this.fillGraph = [];
        this.fillGraph.push({x: 0, y: 0});
        for (let i = this.xBegin; i < this.xEnd; i++) {
            // this.ctx.lineTo(i * this.xBins, 10 - (points[i] * this.yBins) );
            // this.fillGraph.push({x: i * this.xBins, y: 10 - (points[i] * this.yBins)});
            let yVal = -(points[i] * this.yBins * this.points.length + this.lineOffset);
            this.ctx.lineTo((i - this.xBegin) * this.xBins, yVal);
            this.fillGraph.push({x: (i - this.xBegin) * this.xBins, y: yVal});
        }
        // hard coded cap for now (not 800), should be xBins * number of bins
        this.fillGraph.push({x: 800, y: 0});
        // this.ctx.strokeStyle = "#c800ff";
        this.ctx.strokeStyle = this.lineColor;
        this.ctx.stroke();
        this.drawFill();
        this.ctx.restore();
    }

    drawGrid() {
        this.computeDims();
        this.axes();
    }

    axes() {
        if (debug) console.log(`Drawing lines <${this.linesX},${this.linesY}>`);
        for (let i = 0; i <= this.linesX; i++) {
            this.ctx.beginPath();
            this.ctx.lineWidth = 1;
            
            // If line represents X-axis draw in different color
            (i == this.xAxis) ? this.ctx.strokeStyle = "#ffffff" : this.ctx.strokeStyle = "#333333";
            
            if (i == this.linesX) {
                this.ctx.moveTo(0, this.size*i);
                this.ctx.lineTo(this.width, this.size*i);
            } else {
                this.ctx.moveTo(0, this.size*i+0.5);
                this.ctx.lineTo(this.width, this.size*i+0.5);
            }
            this.ctx.stroke();
        }

        // Draw grid lines along Y-axis
        for (let i = 0; i <= this.linesY; i++) {
            this.ctx.beginPath();
            this.ctx.lineWidth = 1;
            
            // If line represents Y-axis draw in different color
            (i == this.yAxis) ? this.ctx.strokeStyle = "#ffffff" : this.ctx.strokeStyle = "#333333";
            
            if(i == this.linesY) {
                this.ctx.moveTo(this.size*i, 0);
                this.ctx.lineTo(this.size*i, this.height);
            } else {
                this.ctx.moveTo(this.size*i+0.5, 0);
                this.ctx.lineTo(this.size*i+0.5, this.height);
            }
            this.ctx.stroke();
        }

        // for RSI 
        this.ctx.beginPath();
        this.ctx.lineWidth = 1;
        this.ctx.strokeStyle = 'white';

        this.ctx.setLineDash([3,5]);
        this.ctx.moveTo(0, (this.height - this.size) * 0.7);
        this.ctx.lineTo(this.width, (this.height - this.size) * 0.7);

        this.ctx.moveTo(0, (this.height - this.size) * 0.3);
        this.ctx.lineTo(this.width, (this.height - this.size) * 0.3);

        this.ctx.stroke();
        this.ctx.setLineDash([0,0]);

        this.ctx.save();
        this.ctx.translate(this.yAxis * this.size, this.xAxis * this.size);
        this.labels();
        this.ctx.restore();
    }

    labels() {
        // Ticks marks along the positive X-axis
        for(let i = 1; i < (this.linesY - this.yAxis); i += 5) {
            this.ctx.beginPath();
            this.ctx.lineWidth = 1;
            this.ctx.strokeStyle = "#eeeeee";

            // Draw a tick mark 6px long (-3 to 3)
            this.ctx.moveTo(this.size*i+0.5, -3);
            this.ctx.lineTo(this.size*i+0.5, 3);
            this.ctx.stroke();

            // Text value at that point
            this.ctx.font = '9px Arial';
            this.ctx.fillStyle = 'white';
            this.ctx.textAlign = 'start';
            this.ctx.fillText(this._labels[i], this.size*i-2, 15);
        }

        // Ticks marks along the negative X-axis
        for(let i = 1; i < this.yAxis; i++) {
            this.ctx.beginPath();
            this.ctx.lineWidth = 1;
            this.ctx.strokeStyle = "#eeeeee";

            // Draw a tick mark 6px long (-3 to 3)
            this.ctx.moveTo(-this.size*i+0.5, -3);
            this.ctx.lineTo(-this.size*i+0.5, 3);
            this.ctx.stroke();

            // Text value at that point
            this.ctx.font = '9px Arial';
            this.ctx.fillStyle = 'white';
            this.ctx.textAlign = 'end';
            this.ctx.fillText(-this.xStart.number*i + this.xStart.suffix, -this.size*i+3, 15);
        }

        // Ticks marks along the positive Y-axis
        // Positive Y-axis of graph is negative Y-axis of the canvas
        for(let i = 1; i < (this.linesX - this.xAxis); i++) {
            this.ctx.beginPath();
            this.ctx.lineWidth = 1;
            this.ctx.strokeStyle = "#eeeeee";

            // Draw a tick mark 6px long (-3 to 3)
            this.ctx.moveTo(-3, this.size*i+0.5);
            this.ctx.lineTo(3, this.size*i+0.5);
            this.ctx.stroke();

            // Text value at that point
            this.ctx.font = '9px Arial';
            this.ctx.textAlign = 'start';
            this.ctx.fillText(-this.yStart.number*i + this.yStart.suffix, 8, this.size*i+3);
        }

        // Ticks marks along the negative Y-axis
        // Negative Y-axis of graph is positive Y-axis of the canvas
        for(let i = 1; i < this.xAxis; i++) {
            this.ctx.beginPath();
            this.ctx.lineWidth = 1;
            this.ctx.strokeStyle = "#eeeeee";

            // Draw a tick mark 6px long (-3 to 3)
            this.ctx.moveTo(-3, -this.size*i+0.5);
            this.ctx.lineTo(3, -this.size*i+0.5);
            this.ctx.stroke();

            // Text value at that point
            this.ctx.font = '9px Arial';
            this.ctx.textAlign = 'start';
            this.ctx.fillText(this.yStart.number*i + this.yStart.suffix, 8, -this.size*i+3);
        }
    }

    draw() {
        // let frameX = document.getElementById(`grid-frame-${this._frame}`); 
        // frameX.append(this.canvas);
        this.canvas.style.display = 'block';
        if (debug) console.log("Drawing");
        this.ctx.clearRect(0, 0, this.width, this.height);
        if (this.type != "pie") this.drawGrid();
        if (this.type == "bar") {
            this.drawBar(this.pointsNorm);
        } else if (this.type == "pie") {
            this.drawPie();
        } else if (this.type == "line") {
            this.drawLine(this.pointsNorm);
        } else if (this.type == "doubleBar") {
            this.drawDoubleBar(this.pointsNorm);
        }
        this.updateReady = false;
        this.drawHud();
        this.drawTooltip();
        this.peripherals();
    }

    drawHud() {
        this.ctx.fillStyle = 'white';
        this.ctx.font = '16px Arial';
        this.ctx.textAlign = 'start';
        this.ctx.fillText(this.ticker.toUpperCase(), this.width - 60, 20);
        this.ctx.font = '14px Arial';
        this.ctx.fillStyle = '#aaaaaa';
        this.ctx.fillText(this.parameter.toLowerCase(), this.width - 60, 40);
    }

    peripherals() {
        if (this.state.pressed) {
            this.ctx.lineWidth = 1;
            this.ctx.rect(this.state.lastPos[0], this.state.lastPos[1], this.cursor.x - this.state.lastPos[0], this.cursor.y - this.state.lastPos[1]);
            this.ctx.fillStyle = "#00ffff44";
            this.ctx.strokeStyle = "#00ffffff";
            this.ctx.fill();
            this.ctx.stroke();
        }

        let ogStyle = this.ctx.strokeStyle;
        this.ctx.beginPath();
        this.ctx.lineWidth = 1;
        this.ctx.strokeStyle = 'white';

        this.ctx.setLineDash([1,3]);
        this.ctx.moveTo(0, this.cursor.y);
        this.ctx.lineTo(this.width, this.cursor.y);

        this.ctx.moveTo(this.cursor.x, 0);
        this.ctx.lineTo(this.cursor.x, this.height);

        this.ctx.stroke();
        this.ctx.setLineDash([0, 0]);
    }

    resize(height, width) {
        this.height = height;
        this.width = width;
        this.draw();
    }   

    min(input) {
        let min = 9999999999;
        for (let i = 0; i < input.length; i++) {
            if (input[i] < min) { min = input[i] };
        }
        return min;
    }

    max(input) {
        let max = -999;
        for (let i = 0; i < input.length; i++) {
            if (input[i] > max) { max = input[i] };
        }
        return max;
    }

    minMaxNorm(input) {
        let result = input;
        let max = this.max(input);
        let min = this.min(input);
        for (let i = 0; i < input.length; i++) {
            result[i] = ((input[i] - min) / (max - min)).toFixed(5);
        }
        return result;
    }

    seed(type, json) {
        switch(type) {
            // specific to 1m (1min) rn ?
            case "historical":
                this.points = [];
                let length = json.length;

                console.log("%c KUNTA KINTE", "color: crimson;");
                console.log(json);

                this.points = [];
                this._labels = [];
                this.volume = [];
                this.transactions = [];
                let lastValidIdx = 0;
                for (let i = 0; i < length; i++) {
                    // console.log(json[i].average);
                    if (json[i].average != null) {
                        lastValidIdx = i;
                        this.points.push(json[i].close);
                        this._labels.push(json[i].label);
                        this.volume.push(json[i].volume);
                        this.transactions.push(json[i].numberOfTrades);
                        this.rawDataX.push(json[i].minute);
                        if (this._parameter == "volume") {
                            this.rawDataY.push(json[i].volume);
                        } else {
                            this.rawDataY.push(json[i].close);
                        }
                    } else {
                        // this.points.push(json[lastValidIdx].close);
                        // this._labels.push(json[lastValidIdx].label);
                        // this.volume.push(json[lastValidIdx].volume);
                        // this.transactions.push(json[lastValidIdx].numberOfTrades);
                        // this.rawDataX.push(json[lastValidIdx].minute);
                        // if (this._parameter == "volume") {
                        //     this.rawDataY.push(json[lastValidIdx].volume);
                        // } else {
                        //     this.rawDataY.push(json[lastValidIdx].close);
                        // }
                    }
                }
                let yMax, yMin;
                yMin = this.max(this.points);
                yMax = this.min(this.points);
                if (this._parameter == "volume") {
                    this.yLabel = "Volume (K)";
                    // this.rawDataY = this.volume;
                    this.pointsNorm = this.minMaxNorm(this.volume);
                } else if (this._parameter == "rsi") { 
                    this.yLabel = "RSI";
                    let newPoints = Algo.RSI(this.points);
                    this.rawDataY = newPoints;
                    this.points = newPoints;
                    this.pointsNorm = this.minMaxNorm(newPoints);
                } else if (this._parameter == "pie") {
                    this.yLabel = "Loss";
                    this.xLabel = "Gain";
                    let weights = Algo.HFA(this.points);
                    this.weightA = weights[0];
                    this.weightB = weights[1];
                    this.rawDataX = [(this.weightA / (this.points.length) * 100).toFixed(2)];
                    this.rawDataY = [(this.weightB / (this.points.length) * 100).toFixed(2)];
                    this.points = [];
                    // this.pointsNorm = this.minMaxNorm(newPoints);  
                } else {
                    this.yLabel = "Price ($)";
                    this.pointsNorm = this.minMaxNorm(this.points);
                    // this.rawDataY = this.points;
                }
                break;
            case "real-time":
                break;
            case "projection":
                break;
            case "SIM":
                break;
            case "CAPM":
                break;
            default:
                break;
        }
    }

    tick() {
        this.ctx = document.getElementById(this.id).getContext('2d');
        if (this.highlighting) {
            this.highlighting = false;
        }
    }

    toggle() {
        if (this.canvas == undefined) {
            console.error("Can not toggle graph on undefined canvas!");
        }
        if (this.canvas.style.display == 'block') {
            this.canvas.style.display = 'none';
        } else if (this.canvas.style.display == 'none') {
            this.canvas.style.display = 'block';
        }
    }

    loop() {
        this.tick();
        this.draw();
        requestAnimationFrame(this.loop.bind(this));
    }

    handler() {
        if (debug) console.log("ID IS: "+this.id);
        document.getElementById(this.id).addEventListener("mousedown", (e) => {
           this.state.pressed = true; 
           this.state.lastPos[0] = this.cursor.x;
           this.state.lastPos[1] = this.cursor.y;
           if (debug) console.log("Pressing canvas: "+this.id);
        });

        document.getElementById(this.id).addEventListener("mouseup", (e) => {
            this.state.pressed = false;
        });

        document.getElementById(this.id).addEventListener('mousemove', (e) => {
            this.active = true;
            if (debug) console.log("mousing over!");
            this.canvas.focus();
            let ctxBnd = this.canvas.getBoundingClientRect();
            this.cursor.x = (e.clientX - ctxBnd.left);
            this.cursor.y = (e.clientY - ctxBnd.top);
            if (debug) console.log(`Cursor is at <${this.cursor.x}, ${this.cursor.y}>`);
        });

        document.getElementById(this.id).addEventListener('mouseleave', (e) => {
            this.active = false;
        });


        document.getElementById(this.id).addEventListener("wheel", (e) => {
            e.preventDefault();
            let zoomGranularity = 0.02;
            if (e.deltaY < 0) {
                this.zoomY -= zoomGranularity;
                if (this.zoomX > 0) {
                    this.zoomX -= 1;
                }
        
                if (this.zoomY < 0) {
                    this.zoomY = 0;
                }
        
            } else if (e.deltaY > 0) {
                this.zoomY += zoomGranularity;
                this.zoomX += 1;
                if (this.zoomY > 1) {
                    this.zoomY = 1;
                }
        
            }
            if (this.zoomX > 0) {
                this.xBegin = this.zoomX;
                this.xEnd = this.points.length - this.zoomX;
            }
        })
    }

    init() {
        this.handler();
        this.drawGrid();
        this.drawLine(this.pointsNorm);
    }

    start() {
        this._running = true;
        this.loop();
    }

    stop() {
        this._running = false;
    }

    get running() { return this._running; }
    set running(val) { this._runnig = val; }

    setPosition(x, y) {
        this.canvas.style.left = `${x}px`;
    }

    get canvas_() { return this.canvas.toObject(); }

    get label() { return this._label; }
    set label(val) { this._label = val; }

    get subLabel() { return this._subLabel; }
    set subLabel(val) { this._subLabel = val; }

    get parameter() { return this._parameter; }
    set parameter(val) { this._parameter = val; }

} export { Graph };

    // average: 21.422
    // changeOverTime: null
    // close: 22.02
    // date: "2022-03-30"
    // high: 22.02
    // label: "09:40 AM"
    // low: 22.011
    // marketAverage: 22.09
    // marketChangeOverTime: 0.01023650599635458
    // marketClose: 21.4
    // marketHigh: 21.7587
    // marketLow: 21.706
    // marketNotional: 660637.37
    // marketNumberOfTrades: 195
    // marketOpen: 21.975
    // marketVolume: 30860
    // minute: "09:40"
    // notional: 15260.7
    // numberOfTrades: 7
    // open: 22.018
    // volume: 715