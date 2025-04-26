#include <iostream>
#include <cstdint>
#include <vector> 
#include <omp.h>
#include <sstream>
#include <stack>
#include "httplib.h"

using namespace std;

struct rgb{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;


    rgb(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(g){}
    rgb(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : red(r), green(g), blue(b), alpha(a){}
    rgb() : red(0), green(0), blue(0){}
};

struct pixel{
    rgb px;

    pixel() : px(){}
    pixel(rgb RGB) : px(RGB){}

    void setPixelValue(int redValue, int greenValue, int blueValue, int alphaValue){
        px.red = validPixelValue(redValue);
        px.green = validPixelValue(greenValue);
        px.blue = validPixelValue(blueValue);
        px.alpha = validPixelValue(alphaValue);
    }

    void setPixel(const pixel& pxl){
        px.red = pxl.px.red;
        px.green = pxl.px.green;
        px.blue = pxl.px.blue;
        px.alpha = pxl.px.alpha;
    }

    void rgbTo8Bit(){
        px.red = (px.red + px.green + px.blue)/3;
    }

    void rgbTo1Bit(int threshold){
        px.red >= threshold ? px.red = 255 : px.red = 0; 
    }

    void adaptativeThresholdSync(vector<vector<pixel>>& img, int padding, int targetY, int targetX, int pixelAmount)  { 
        int value;
        value = 0;

        for(int i = targetY-padding; i <= targetY+padding; i++){
            for (int j = targetX-padding; j <= targetX+padding; j++){
                value += img[i][j].px.red;
            }
        }
        
        uint8_t threshold = validPixelValue(value/pixelAmount);

        px.red = px.red > threshold ? 255 : 0;

        px.alpha = 255;    
    }
    
    void adaptativeThresholdAsync(vector<vector<pixel>>& img, int padding, int targetY, int targetX, int pixelAmount)  { 
        int value;
        value = 0;

        for(int i = targetY-padding; i <= targetY+padding; i++){
            for (int j = targetX-padding; j <= targetX+padding; j++){
                value += img[i][j].px.red;
            }
        }
        
        uint8_t threshold = validPixelValue(value/pixelAmount);

        px.red = px.red > threshold ? 255 : 0;

        px.alpha = 255;    
    }
    uint8_t validPixelValue(int value){
        if(value>255){
            return 255;
        }
        if(value < 0){
            return 0;
        }
        return value;
    }

};

vector<vector<pixel>> convertTo8Bit(vector<vector<pixel>>& imgMatrix){

    for(size_t i = 0; i < imgMatrix.size(); i++){
        for(size_t j = 0; j < imgMatrix[i].size(); j++){
            imgMatrix[i][j].rgbTo8Bit();
        }
    }

    return imgMatrix;
}


vector<vector<pixel>> convolutionalSync(vector<vector<pixel>>& img, vector<vector<pixel>>& imgResult, int padding, int area){
    
    for(size_t i = padding; i < imgResult.size() + padding; i++){
        for(size_t j = padding; j < imgResult[0].size() + padding; j++){
            imgResult[i-padding][j-padding].adaptativeThresholdSync(img, padding, i, j, area);
        }
    }

    return imgResult;
}


vector<vector<pixel>> convolutionalAsync(vector<vector<pixel>>& img, vector<vector<pixel>>& imgResult, int padding, int area){


    #pragma omp parallel for shared(imgResult)
    for(size_t i = padding; i < imgResult.size() + padding; i++){
        for(size_t j = padding; j < imgResult[0].size() + padding; j++){
            imgResult[i-padding][j-padding].adaptativeThresholdAsync(img, padding, i, j, area);
        }
    }

    return imgResult;
}

vector<vector<pixel>> edgeAdjust(vector<vector<pixel>>& img, int kernel){
    size_t originalRows = img.size();
    size_t originalColums = img[0].size();
    int padding = kernel / 2;
    vector<vector<pixel>> imgResult(originalRows + kernel, vector<pixel>(originalColums + kernel));
    pixel pxl;
    pxl.px.alpha = 255;

    for(size_t i = 0; i < imgResult.size(); i++){
        for(size_t j = 0; j < imgResult[0].size(); j++){
            if (i < padding) { // Borda superior
                if (j < padding) { // Canto superior esquerdo
                    pxl = img[0][0];
                } else if (j >= originalColums + padding) { // Canto superior direito
                    pxl = img[0][originalColums - 1];
                } else { // Borda superior (meio)
                    pxl = img[0][j - padding];
                }
            } else if (i >= originalRows + padding) { // Borda inferior
                if (j < padding) { // Canto inferior esquerdo
                    pxl = img[originalRows - 1][0];
                } else if (j >= originalColums + padding) { // Canto inferior direito
                    pxl = img[originalRows - 1][originalColums - 1];
                } else { // Borda inferior (meio)
                    pxl = img[originalRows - 1][j - padding];
                }
            } else if (j < padding) { // Borda esquerda (sem ser os cantos)
                pxl = img[i - padding][0];
            } else if (j >= originalColums + padding) { // Borda direita (sem ser os cantos)
                pxl = img[i - padding][originalColums - 1];
            } else { // Região central (imagem original)
                pxl = img[i - padding][j - padding];
            }
            imgResult[i][j].setPixel(pxl);
        }
    }

    return imgResult;
}


// ---------------------------------------------------------------------------------------------

// Transformando pixel para string
string pixelToString(const pixel& p, int cont, int height) {
    stringstream rgbString;

    if (cont < height) {
        rgbString << "\n[" << (int)p.px.red << "],\n";
    }
    else{
        rgbString << "\n[" << (int)p.px.red << "]\n";
    }
    
    return rgbString.str();
}

// Transformando matriz de pixeis para string -- JSON
string imgToString(const vector<vector<pixel>>& image) {
    ostringstream result;
    string imgSTR;
    int height = image.size();
    int width = image.empty() ? 0 : image[0].size();
    int cont = 0;
    result << "[" << "\n";

    for (const auto& row : image) {
	result << "[";
        for (const auto& px : row) {
            cont = (cont==width) ? 1 : ++cont;
            result << pixelToString(px, cont, width); // Adiciona a string do pixel
        }
	result << "],\n";// Nova linha após cada linha de pixels
    }

    result << "]" << "\n";

    imgSTR = result.str();

    imgSTR.erase(imgSTR.size() -4, 2);

    return imgSTR;
}

// Transformando string para pixel
pixel transformStringtoRGBA(string& stringValue){
    string STRvalue;
    uint8_t valueRGB[4] = {0,0,0,255};
    stringstream ss(stringValue);
    char ch;
    int cont =0;

    while(ss >> ch){

        if(isdigit(ch)){
            STRvalue += ch;
        }else{
            int integer = stoi(STRvalue);
            valueRGB[cont] = static_cast<uint8_t>(integer);
            STRvalue = "";
            cont++;
        }
    }
    
    rgb RGB = {valueRGB[0], valueRGB[1], valueRGB[2], valueRGB[3]};

    pixel px(RGB);

    return px;
}


//Transformando JSON string para matriz de pixeis [imagem]
vector<vector<pixel>> parse_json_pixels(string& body){
    vector<vector<pixel>> pixels;
    vector<pixel> row;
    string stringValue;
    stack<char> stack;
    
    //Remover '[ ]' de abertura e de fechamento
    body.erase(0,1);
    body.erase(body.size() -1,1);

    stringstream ss(body);
    char ch;

    size_t pos = 0;

    while(ss >> ch) {   /// PILHA [, 
        //Entrou na coluna
        if (ch == '['){
            stack.push(ch);

            while(!stack.empty()){
                ss >> ch;
                if(ch == '['){
                    stack.push(ch);
                }else if (ch == ']'){
                    pixels.push_back(row);
                    row.clear();
                    stringValue = "";
                    stack.pop();
                }else if (isdigit(ch)){
                    while(ch != ',' && ch != ']'){
                        stringValue += ch;
                        ss >> ch;
                    }
                    if(ch==','){
                        stringValue += ch;
                    }else{
                        pixel px = transformStringtoRGBA(stringValue);
                        row.push_back(px);
                        stringValue = "";
                        if(!stack.empty()){                        
                            stack.pop();
                        }
                    }
                    
                }
            }
        }
    }

    return pixels;
}




int main() {

    httplib::Server svr;


    svr.set_default_headers({
        {"Access-Control-Allow-Origin", "*"} // Permite todas as origens
    });


    svr.Post("/sync/threshold", [](const httplib::Request& req, httplib::Response& res){
        if(!req.has_param("kernel") || !req.has_param("value")){
            res.set_content(req.body, "application/json");
            return;
        }

        int kernel = stoi(req.get_param_value("kernel"));
        int value = stoi(req.get_param_value("value"));

        string body = req.body;
        vector<vector<pixel>> img = parse_json_pixels(body);

        vector<vector<pixel>> result = edgeAdjust(img, kernel);

        double inicio, fim;
        int area = kernel * kernel;


        // Sincrona
        cout << "Execução Síncrona" << endl;
        inicio = omp_get_wtime();
        for (int i = 0; i<=value; i++){
            img = convolutionalSync(result, img, kernel/2, area);
            cout << i << endl;
        }
        fim = omp_get_wtime();

        double time = fim - inicio;

    
        string responseIMG = imgToString(img);
    
        ostringstream json;
        json    << "{\n \"image\": " << responseIMG 
                << ",\n \"time\": " << time << "\n}";

        res.set_content(json.str(), "application/json");
    });

    svr.Post("/async/threshold", [](const httplib::Request& req, httplib::Response& res){
        if(!req.has_param("kernel") || !req.has_param("value")){
            res.set_content(req.body, "application/json");
            return;
        }

        int kernel = stoi(req.get_param_value("kernel"));
        int value = stoi(req.get_param_value("value"));

        string body = req.body;
        vector<vector<pixel>> img = parse_json_pixels(body);

        vector<vector<pixel>> result = edgeAdjust(img, kernel);


        double inicio, fim;
        int area = kernel* kernel;


        cout << "Execução Assíncrona" << endl;
        //Assincrona
        inicio = omp_get_wtime();
        for (int i = 0; i<=value; i++){
            img = convolutionalAsync(result, img, kernel/2, area);
            cout << i << endl;
        }
        fim = omp_get_wtime();

        double time = fim - inicio;

    
        string responseIMG = imgToString(img);

        ostringstream json;
        json    << "{\n \"image\": " << responseIMG 
                << ",\n \"time\": " << time << "\n}";

        res.set_content(json.str(), "application/json");
    });

    
    svr.listen("localhost", 8080);

    return 0;
}