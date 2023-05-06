import UI from "../../jsm/nebula/polaris/node_editor.js";
import * as Polaris from "../../jsm/nebula/polaris/simple_graph.js";
let nodeEditor = new UI.JerichoEditor("db-container", "db-canvas", "db-ui", 1000, 600, undefined);
let g = new Polaris.Graph(300, 300, `graph-latency-example`);
let g2 = new Polaris.Graph(300, 300, `graph-rounds-example`, "bar");
let g3 = new Polaris.Graph(300, 300, `graph-accuracy-example`);
g.init(); g2.init(); g3.init();
g.start(); g2.start(); g3.start();
let xs = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'];
let ys = [1, 3, 4, 5, 2, 1, 4, 5,3, 5];
let xs2 = ['a', 'b', 'c', 'd'];
let ys2 = [3, 8, 4, 2];
let xs3 = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
let ys3 = [0.1, 0.4, 4, 0.8, 3, 0.9, 0.92, 5, 0.924, 0.925];

let graphs = {};

setTimeout(function() {
    g.update(xs,ys);
}, 5000);

setTimeout(function() {
    g.update(xs2,ys2);
}, 8000);

setTimeout(function() {
    g.update(xs,ys2);
}, 9000);

g2.update(xs3, ys3);

    let socket = undefined;
// document.addEventListener("DOMContentLoaded", function(event) { 
    socket = socketTest();
    let open = false;
    document.getElementById("web-socket").addEventListener("click", function() {
        if (!open) {
            socketTest();
            open = true;
        } else {
            socket.close(1000);
            open = false;
        }
    });

    document.getElementById("ws-init").addEventListener("click", function(){
        // if (socket == undefined) {
            console.log("%c Socket is undefined", "color: green;");
            socketTest();
            let s = JSON.stringify({route: "new", function: "main"});
            socket.send(s);
        // } else {
        //     console.log("%c Socket is defined", "color: red;");
        // }
    });

    document.getElementById("send-socket").addEventListener("click", function() {
        let s = JSON.stringify({route: "new", function: "main"});
        socket.send(s);
    });

    function socketTest() {
        socket = new WebSocket("wss://localhost:8080");
        socket.open = () => { socket.send("{hello: \"Message to send.\"}"); };
        socket.onmessage = (e) => {
            // console.log("MESSAGE RECEIVED");
            let received = e.data;
            console.log(`%c Message Received: ${received}`, "color: green");
            console.log(received.length);
            console.log(received[145]);
            if (received[0] == "{") {
                let json = JSON.parse(received);
                if (typeof json === "object" && json != null) {
                    let command = json.command;
                    let message = parseFloat(json.message);
                    let hostname = json.obj.hostname;
                    switch (command) {
                        case "accuracy":
                            nodeEditor.accuracy(hostname, 1, message);
                            break;
                        case "latency":
                            nodeEditor.latency(hostname, 1, message);
                            break;
                        case "rounds":
                            nodeEditor.rounds(hostname, 1, message);
                            break;
                        case "bandwidth-in":
                            nodeEditor.bandI(hostname, 1, message);
                            break;
                        case "bandwidth-out":
                            nodeEditor.bandO(hostname, 1, message);
                            break;
                        case "state":
                            nodeEditor.state(hostname, json.message);
                            break;
                        case "tolerant-state":
                            nodeEditor.state(json.obj.url, json.obj.state);
                            break;
                        case "node-connect":
                            nodeEditor.connect(json.message, hostname);
                            break;
                        case "node-disconnect":
                            nodeEditor.disconnect(json.message, hostname);
                            break;
                        case "final-latency":
                            nodeEditor.finalLatency(hostname, message);
                            break;
                        case "final-train":
                            nodeEditor.finalTrain(hostname, message);
                            break;
                        case "final-rounds":
                            nodeEditor.finalRounds(hostname, message);
                            break;
                        default:
                            break;
                    }
                } else {
                    console.log(`%c Not an object: ${received}`, "color: red");
                }
            }
        };

        socket.onclose = () => { console.log("Connection is closed."); }
        socket.onerror = (error) => { console.log(error); }
    }
// });

$("#reset-script").click(function(){
    nodeEditor.nodes.forEach((el) => {
        $.ajax({
            method: "GET",
            url: el.url + "/reset-federator",
            success: function(e) {
                console.log('Successfully requested stop!');s
            }
        });
    });
});

$("#node-create").click(function(){
    let url = $("#node-url").val();
    let role = $("#node-role").val();
    console.log(role);
    console.log(url);
    nodeEditor.createNode("127.0.0.1"+url, role);
});

$("#node-editor-output").click(function(){
    console.log("Adding node!");
    nodeEditor.addNode("output");
});

$("#node-editor-index").click(function(){
    console.log("Adding node!");
    nodeEditor.addNode("index");
});

$("#node-editor-new").click(function(){
    nodeEditor.clear();
});

$("#node-editor-function").click(function(){
    console.log("Generating function!");
    nodeEditor.addNode("function");
});

$("#node-editor-load").click(function(){
    console.log("Loading editor!");
    $("#modal-depth").val(2);
    $("#stocket-load-directory").children().each(function(){
        $(this).remove();
    });
    $("#stocket-load-modal").show();
    nodeEditor.load();
});

nodeEditor.init();
nodeEditor.run();

$("#save-script").on("click", function(){
    let name = $("#save-name").val();
    if (name != "") {
        nodeEditor.scriptName = name + "-gui";
    }
    nodeEditor.saveJericho();
});

$("#run-script").on("click", function(){
    nodeEditor.run();
});

$(document).ready(function(){
    let poll_gate = undefined;
    $.ajaxSetup({ cache: false });
    $("[data-role=tab]").each(function(){
        if ($(this).attr('default') === undefined) {
            $(this).hide();
        }
    });
    $("[data-role='tab-trigger']").on("click", function(){
        let target = $(this).attr('data-target');
        let self = $(this);
        $("[data-role=tab]").each(function(){
            $(`${target}[data-role=tab]`).css('background-color', '#666');
            $(this).hide();
        });
        $("[data-role='tab-trigger']").each(function(){
            if ($(this).hasClass("active")) {
                $(this).removeClass("active");
            }
        });
        $(`${target}[data-role=tab]`).show();
        self.addClass("active");
    });

    $("[data-role='request']").on("click", function(){
        let request = $(this).attr('data-request');
        let attrs = $(this).attr('data-attrs');
        let token = $("#token").val();
        let unique = $(this).attr('data-unique');
        let attr_arr = []
        if (attrs != undefined) {
            attr_arr = attrs.split(":");
        }
        let arr = request.split("*");
        let response = request;
        if ($(this).attr('data-target') !== undefined) {
            response = $(this).attr('data-target');
        }
        console.log(`attr arrr is ${attr_arr}, size is ${attr_arr.length}`);
        console.log(`arrr data is ${arr[2]}`);
        if (arr.length == 4) {
            poll_gate = arr[3];
        }
        let json = JSON.parse(arr[2]);
        json.token = token;
        if (unique != undefined) {
            attr_arr.forEach((el) => {
                json[el] = $(`[data-unique='${unique}'][data-attr='${el}']`).val();
            });
        } else {
            attr_arr.forEach((el) => {
                json[el] = $(`[data-attr=${el}]`).val();
            });
        }
        console.log(`json string is ${JSON.stringify(json)}`);
        console.log(arr[1]);
        console.log(json);
        poll(arr[1], arr[0], json, response);
    });

        $("[data-role=modal-trigger]").each(function(){
            $(this).on('click', function(){
            // if ($(this).attr('data-type', "modal")) {
                let target = $(this).attr('data-target');
                console.log(" targetoo ");
                console.log(target);
                $(`${target}`).show();
                let depth = $(`${target}`).attr('data-depth');
                if (depth == undefined) { depth = 1; }
                $("#modal-depth").attr('value', depth);
                $(`${target}`).attr('data-depth', depth);
                console.log($(this).attr("id"));
                if ($(this).attr("id") == "load-scripts") {
                    $.ajax({
                        method: "GET",
                        url: "/list-scripts",
                        success: function(e) {
                            $("#script-list").empty();
                            console.log(e);
                            let arr = JSON.parse(e);
                            arr.forEach((el) => {
                                let li = document.createElement("li");
                                li.setAttribute("data-role", "load-script");
                                li.setAttribute("data-script", el);
                                li.innerText = el;
                                li.addEventListener("click", function(){
                                    let x = function() {
                                        return $.ajax({
                                            method: "POST",
                                            url: "/load-script",
                                            data: el,
                                            success: function(e) {
                                                socketTest();
                                                $("div#tab-collection").children().each(function(){
                                                    console.log(this);
                                                    let role = this.id;
                                                    console.log(role);
                                                    if (role.includes("8")) {
                                                        $(`#${role}`).remove();
                                                    }
                                                });
                                                $("#tab-triggers").children().each(function(){
                                                    let target = this.getAttribute('data-target');
                                                    let id = this.id;
                                                    console.log(target);
                                                    console.log(id);
                                                    if (target.includes("8")) {
                                                        $(`#${id}`).remove();
                                                    }
                                                });
                                                let json = JSON.parse(e);
                                                let nodes = json.nodes;
                                                let edges = json.edges;
                                                nodeEditor.jerichoScript(nodes, edges);       
                                                nodeEditor.nodes.forEach((el) => {
                                                    console.log("%c SMD", "color:magenta");
                                                    // let el = nodes[n];
                                                    console.log(el);
                                                    let url = el.url;
                                                    let port = url.split(":")[1];
                            
                            
                                                    // <li style="border-bottom:1px solid black;padding:5px;" class="hover-bg-white" data-role="tab-trigger" data-target="#polygon">Polygon</li>
                                                    let trigger = document.createElement("li");
                                                    trigger.style.borderBottom = "1px solid black;";
                                                    trigger.style.padding = "5px";
                                                    trigger.id = port+"-trigger";
                                                    trigger.className = "hover-bg-white";
                                                    trigger.setAttribute("data-role", "tab-trigger");
                                                    trigger.setAttribute("data-target", `#${port}`);
                                                    trigger.innerText = url;
                                                    document.getElementById("tab-triggers").append(trigger);
                            
                                                    // <div id="postgres" data-role="tab">
                                                    //     Welcome to nebula postgres
                                                    // </div>
                                                    let tab = document.createElement("div");
                                                    tab.setAttribute("id", port);
                                                    tab.setAttribute("data-role", "tab");
                        
                                                    let gm = document.createElement("div");
                                                    gm.setAttribute("id", `graph-container-${port}`);
                                                    let acc = document.createElement("div");
                                                    acc.setAttribute("id", `graph-accuracy-${port}`);
                                                    let bandIn = document.createElement("div");
                                                    bandIn.setAttribute("id", `graph-band-in-${port}`);
                                                    let bandOut = document.createElement("div");
                                                    bandOut.setAttribute("id", `graph-band-out-${port}`);
                                                    let latency = document.createElement("div");
                                                    latency.setAttribute("id", `graph-latency-${port}`);
                                                    let final = document.createElement("div");
                                                    final.setAttribute("id", `final-stats-${port}`);
                                                    
                                                    gm.append(acc); 
                                                    gm.append(bandIn); 
                                                    gm.append(bandOut); 
                                                    gm.append(latency); 
                                                    gm.append(final);

                                                    tab.style.display = "none";
                                                    tab.append(gm);
                                                    document.getElementById("tab-collection").append(tab);
                                                    let a = new Polaris.Graph(300, 300, `graph-latency-${port}`, "line", "Latency");
                                                    let b = new Polaris.Graph(300, 300, `graph-band-in-${port}`, "bar", "Band In");
                                                    let d = new Polaris.Graph(300, 300, `graph-band-out-${port}`, "bar", "Band Out");
                                                    let c = new Polaris.Graph(300, 300, `graph-accuracy-${port}`, "line", "Accuracy");
                                                    a.init(); a.start();
                                                    b.init(); b.start();
                                                    c.init(); c.start();
                                                    d.init(); d.start();
                                                    let gMap = {
                                                        accuracy: {"graph": c, "aggregates": [[],[]]},
                                                        bandI: {"graph": b, "aggregates": [[],[]]},
                                                        bandO: {"graph": d, "aggregates": [[],[]]},
                                                        latency: {"graph": a, "aggregates": [[],[]]}
                                                    };
                                                    nodeEditor.addMap(url, gMap);
                                                    console.log(`%c URL is ${url}`, "color: orange");
                                                    // nodeEditor.state(url, "joined");
                                                    // nodeEditor.disconnect("127.0.0.1:8081", "127.0.0.1:8080");
                                                    let o = {
                                                        "accuracy": c,
                                                        "latency": a,
                                                        "rounds": b
                                                    };
                                                    graphs[`${port}`] = o;
                            
                                                    $("[data-role=tab]").each(function(){
                                                        if ($(this).attr('default') === undefined) {
                                                            $(this).hide();
                                                        }
                                                    });
                                                    $("[data-role='tab-trigger']").on("click", function(){
                                                        let target = $(this).attr('data-target');
                                                        let self = $(this);
                                                        $("[data-role=tab]").each(function(){
                                                            $(`${target}[data-role=tab]`).css('background-color', '#666');
                                                            $(this).hide();
                                                        });
                                                        $("[data-role='tab-trigger']").each(function(){
                                                            if ($(this).hasClass("active")) {
                                                                $(this).removeClass("active");
                                                            }
                                                        });
                                                        $(`${target}[data-role=tab]`).show();
                                                        self.addClass("active");
                                                    });    
                                                });
                                            }
                                        });
                                    }
                                    let y = function(data, status, jqXHR) {
                                        nodeEditor.state("127.0.0.1:8080", "online");
                                        return $.ajax({
                                            method: "GET",
                                            url: "/ping-all",
                                            success: function(el) {
                                                console.log("WHOOOOOA BUDDY");
                                                console.log(el);
                                                let temp = JSON.parse(el);
                                                console.log("temp.command: "+temp.command);
                                                for (let i = 0; i < temp.responses.length; i++) {
                                                    let o = temp.responses[i];
                                                    let addr = o.url.slice(7);
                                                    addr = addr.split("/")[0];
                                                    console.log("temp.addr: "+addr);
                                                    console.log(`%c msg: ${o.message}`, "color:#ffaaff");
                                                    if (typeof o.message !== "string") {                                                    
                                                        console.log("DORMANT");
                                                    } else {
                                                        let z = JSON.parse(o.message);
                                                        if (z["live"]) {
                                                            nodeEditor.state(addr, "online");
                                                        }
                                                        console.log(`%c ${z["live"]}`, "color:#ccff00");
                                                    }
                                                }
                                            }
                                        }); 
                                    } 
                                    x().then(y);
                                });
                                $("#script-list").append(li);
                            });
                        }
                    })
                }
            // }
            });
        });

        $(window).on('click', function(e){
            console.log("window click");
            console.log(e.target);
            let depth = $("#modal-depth").attr('value');
                if($(e.target).is(".slim.modal")){
                $(".slim.modal").each(function(){
                    if ($(this).attr('data-depth') >= depth) {
                    depth = depth - 1;
                    if (depth < 0) { depth = 0; }
                    console.log("depththhthththt: " + depth);
                    $("#modal-depth").attr('value', depth);
                    $(this).hide();
                    }
                });
                }
        });

    $("#pinger").click(function(){
        $.ajax({
            method: "GET",
            url: "/ping-all",
            success: function(el) {
                console.log("WHOOOOOA BUDDY");
                console.log(el);
            }
        }); 
    });

    function poll(url, method, json, response) {
        $.ajax({
            url: url,
            method: method,
            data: json,
            crossDomain: true,
            // cache: false,
            success: function(data) {
                console.log("Response destination: " + response);
                console.log(data);
                let si = JSON.parse(data);
                console.log(si.status);
                $(`[data-response=${response}]`).text(data);
                let arr_url = url.split("/");
                url = arr_url[arr_url.length - 1];
                if (url == "ping-all") {
                    socketTest();
                    let res = si.response;
                    let ports = Object.keys(res);
                    console.log(ports); 
                    for (let port of ports) {
                        let node = res[port];
                        let active = node.live;
                        let message = node.response;
                        console.log("PORT " + port + "- active: " + active + ", message: " + message);
                        if (active) {
                            $("[data-role=server-node][data-port="+port+"]").css("background-color", "green");
                        } else {
                            $("[data-role=server-node][data-port="+port+"]").css("background-color", "lightgrey");
                        }
                    }
                } else if (url == "request-join") {
                    let res = si.response;
                    let ports = Object.keys(res);
                    console.log(ports); 
                    for (let port of ports) {
                        let node = res[port];
                        let active = node.status;
                        let message = node.message;
                        console.log("PORT " + port + "- active: " + active + ", message: " + message);
                        if (active == "joined") {
                            $("[data-role=server-node][data-port="+port+"]").css("background-color", "blue");
                        } else {
                            $("[data-role=server-node][data-port="+port+"]").css("background-color", "green");
                        }
                    }
                } else if (url == "setup") {
                    let nodes = si.nodes;
                    let edges = si.edges;
                    console.log(nodes);
                    nodeEditor.jerichoScript(nodes, edges);
                    for (let n in Object.keys(nodes)) {
                        let el = nodes[n];
                        console.log(el);
                        let url = el.url;
                        let port = url.split(":")[1];


                        // <li style="border-bottom:1px solid black;padding:5px;" class="hover-bg-white" data-role="tab-trigger" data-target="#polygon">Polygon</li>
                        let trigger = document.createElement("li");
                        trigger.style.borderBottom = "1px solid black;";
                        trigger.style.padding = "5px";
                        trigger.className = "hover-bg-white";
                        trigger.setAttribute("data-role", "tab-trigger");
                        trigger.setAttribute("data-target", `#${port}`);
                        trigger.innerText = url;
                        document.getElementById("tab-triggers").append(trigger);

                        // <div id="postgres" data-role="tab">
                        //     Welcome to nebula postgres
                        // </div>
                        let tab = document.createElement("div");
                        tab.setAttribute("id", port);
                        tab.setAttribute("data-role", "tab");

                        // <div id="graph-container" style="float:left;">
                        // <div id="graph-accuracy-8080">
                        // </div>
                        // <div id="graph-rounds-8080">
                        // </div>
                        // <div id="graph-latency-8080">
                        // </div>
                        // </div>
                        let gm = document.createElement("div");
                        gm.setAttribute("id", `graph-container-${port}`);
                        let acc = document.createElement("div");
                        acc.setAttribute("id", `graph-accuracy-${port}`);
                        let rounds = document.createElement("div");
                        rounds.setAttribute("id", `graph-rounds-${port}`);
                        let latency = document.createElement("div");
                        latency.setAttribute("id", `graph-latency-${port}`);
                        gm.append(acc); gm.append(rounds); gm.append(latency);
                        tab.style.display = "none";
                        tab.append(gm);
                        document.getElementById("tab-collection").append(tab);
                        let a = new Polaris.Graph(300, 300, `graph-latency-${port}`, "line", "Latency");
                        let b = new Polaris.Graph(300, 300, `graph-rounds-${port}`, "bar", "Rounds");
                        let c = new Polaris.Graph(300, 300, `graph-accuracy-${port}`, "line", "Accuracy");
                        a.init(); a.start();
                        b.init(); b.start();
                        c.init(); c.start();
                        let o = {
                            "accuracy": c,
                            "latency": a,
                            "rounds": b
                        };
                        graphs[`${port}`] = o;

                        $("[data-role=tab]").each(function(){
                            if ($(this).attr('default') === undefined) {
                                $(this).hide();
                            }
                        });
                        $("[data-role='tab-trigger']").on("click", function(){
                            let target = $(this).attr('data-target');
                            let self = $(this);
                            $("[data-role=tab]").each(function(){
                                $(`${target}[data-role=tab]`).css('background-color', '#666');
                                $(this).hide();
                            });
                            $("[data-role='tab-trigger']").each(function(){
                                if ($(this).hasClass("active")) {
                                    $(this).removeClass("active");
                                }
                            });
                            $(`${target}[data-role=tab]`).show();
                            self.addClass("active");
                        });

                    };
                }
                if (poll_gate) {
                    setTimeout(function() {
                        poll(url, method, json)
                    }, 2000);
                }
            }
        });
    }
});