document.getElementById('labelRange').innerHTML  = `<label for="range">Valor: 37</label>`;

document.addEventListener("DOMContentLoaded", () => {
    document.getElementById("proccessButton").addEventListener('click', thresholdImage);
    document.getElementById("inputImage").addEventListener('input', loadImageToCanvas);
    document.getElementById("kernelRange").addEventListener('input', updateRange);
});

function loadImageToCanvas() {
    inputId = "inputImage";
    canvasId = "showInputImage";
    document.getElementById(inputId).addEventListener('change', function(event) {
        var file = event.target.files[0];
        if (file) {
            var reader = new FileReader();
            reader.onload = function(e) {
                var img = new Image();
                img.onload = function() {
                    var canvas = document.getElementById(canvasId);
                    var ctx = canvas.getContext('2d');

                    // Ajusta o tamanho do canvas para a imagem
                    canvas.width = 250;
                    canvas.height = 250;

                    // Desenha a imagem no canvas
                    ctx.drawImage(img, 0, 0, canvas.width, canvas.height);
                };
                img.src = e.target.result;
            };
            reader.readAsDataURL(file);
        }
    });
}

function transformIMGtoMATRIX(canvas) {
    var ctx = canvas.getContext('2d');

    // Aguardar a imagem ser carregada antes de processá-la
    var imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
        var data = imageData.data;

        // Criar a matriz
        var matrix = [];
        for (var y = 0; y < canvas.height; y++) {
            var row = [];
            for (var x = 0; x < canvas.width; x++) {
                // Cada pixel é representado por um array [r, g, b, a]
                var index = (y * canvas.width + x) * 4;
                var rgba = [
                    data[index],     
                    data[index + 1], 
                    data[index + 2], 
                    data[index + 3]  
                ];
                row.push(rgba);
            }
            matrix.push(row);
        }

        // Exibir a matriz no formato JSON 
        matrixJSON = JSON.stringify(matrix, null, 2);
        return matrixJSON;
        //JSON ordem: [WIDTH][HEIGHT]
}

function draw8bitImage(imageData){
    var canvas = document.getElementById('outputImage');
    var ctx = canvas.getContext('2d');

    const height = imageData.length;
    const width = imageData[0].length;
    canvas.width = width;
    canvas.height = height;

    const imageDataObj = ctx.createImageData(width, height);

    let i = 0;
    for(let y = 0; y < height; y++){
        for(let x = 0; x < width; x++){
            let pixel = imageData[y][x];
            imageDataObj.data[i++] = pixel;
            imageDataObj.data[i++] = pixel; 
            imageDataObj.data[i++] = pixel; 
            imageDataObj.data[i++] = 255;  
        }
    }

    ctx.putImageData(imageDataObj, 0, 0);   
}

function setExecutionTime(time){
    document.getElementById('executionTime').innerHTML = `<h2>Tempo de execução = ${time}</h2>`
}

function updateRange(){
    const value = document.getElementById('kernelRange').value;
    document.getElementById('labelRange').innerHTML = `<label for="range">Valor: ${value}</label>`;
}

function thresholdImage(){
    const isAsync = document.getElementById("checkbox").checked ? 'async' : 'sync';
    const value = document.getElementById("inputNumber").value;
    const matrixJson = transformIMGtoMATRIX(document.getElementById('showInputImage'));
    const kernel = document.getElementById("kernelRange").value;

    let endpoint = `/api/${isAsync}/threshold?value=${value}&kernel=${kernel}`;

    fetch(endpoint, {
        method:'POST',
        body: matrixJson
    })
    .then(response => response.json())
    .then(data => {
        draw8bitImage(data.image);
        setExecutionTime(data.time);
    })
    .catch(error => {
        console.error('Erro:', error);
    })
}