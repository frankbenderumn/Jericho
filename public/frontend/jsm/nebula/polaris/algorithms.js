export function RSI(data) {
    const sum = (accumulator, curr) => accumulator + curr;
    let result = Array(data.length);
    for (let i = 1; i < data.length; i++) {
        let gains = [];
        let losses = [];
        for (let j = 1; j < i; j++) {
            let diff = (data[j] - data[j - 1]) / data[j - 1];

            // console.log("RSI DIFF: " + diff);

            (diff > 0) ? gains.push(diff) : losses.push(diff);
        }

        let gAvg;
        let lAvg;

        if (gains.length == 0) {
            gAvg = 0;
        } else {
            gAvg = gains.reduce(sum) / gains.length;
        }

        if (losses.length == 0) {
            lAvg = 0;
        } else {
            lAvg = losses.reduce(sum) / losses.length;
        }

        result[i] = (100 - (100 / (gAvg / lAvg))).toFixed(4);
    }
    return result;
}

export function HFA(data) {
    // const sum = (accumulator, curr) => accumulator + curr;
    // let result = Array(data.length);
    let result = [0, 0];
    for (let i = 1; i < data.length; i++) {
        // let gains = [];
        // let losses = [];
        // for (let j = 1; j < i; j++) {
        let diff = (data[i] - data[i - 1]);
        (diff > 0) ? result[0]++ : result[1]++;

            // console.log("RSI DIFF: " + diff);

        //     (diff > 0) ? gains.push(diff) : losses.push(diff);
        // }

        // let gAvg;
        // let lAvg;

        // if (gains.length == 0) {
        //     gAvg = 0;
        // } else {
        //     gAvg = gains.reduce(sum) / gains.length;
        // }

        // if (losses.length == 0) {
        //     lAvg = 0;
        // } else {
        //     lAvg = losses.reduce(sum) / losses.length;
        // }

        // result[i] = 100 - (100 / (gAvg / lAvg));
    }
    return result;
}

export function macd(data) {

}

export function ema(data) {

}

export function bollinger(data) {

}

export function stochOsc(data) {

}