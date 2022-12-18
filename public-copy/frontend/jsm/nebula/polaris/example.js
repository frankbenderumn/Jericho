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