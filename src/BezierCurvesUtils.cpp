#include "BezierCurvesUtils.h"

unsigned int Factorial(unsigned int n){
    if (n == 0)
       return 1;
    return n * Factorial(n - 1);
}

float BernsteinPolynom(unsigned int k, unsigned int n, float t){
    return (Factorial(n)/(Factorial(k)*Factorial(n-k)))*pow(t, k)*pow(1-t, n-k);
}

glm::vec4 PointInBezierCurve(std::vector<glm::vec4> p, float t){

    if(p.size()%4 != 0){
        printf("Piecewise Cubic Bezier Curves devem ser definidas para um número de pontos divisível por 4");
    }

    int numPieces = (int)p.size()/4;

    int currentPiece = (int)t;
    float piecewiseT = t - currentPiece;
    glm::vec4 retorno = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

    for(int i = 1; i<=4; i++){
        retorno += BernsteinPolynom(i-1, 3, piecewiseT)*p[(currentPiece*4)+i-1];
    }

    retorno = glm::vec4(retorno.x, retorno.y, retorno.z, 1.0f);

    return retorno;
}
