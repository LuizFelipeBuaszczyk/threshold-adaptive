# Paralelismo x Sincronismo com algoritmo de Threshold Adaptativo

Esse projeto tem o intuito de mostrar a eficácia de abordagens assíncronas. Neste caso a tecnologia utilizada foi o *OpenMP* para paralelizar um loop for.

---

## Como rodar o projeto?
1. *Primeira Forma - Compilando com g++*
    Este projeto tem como objetivo demonstrar a eficácia de     abordagens assíncronas. Para isso, foi utilizada a tecnologia *OpenMP* para paralelizar um loop for.

    
    g++ -fopenmp -o threshold_adaptive main.cpp -pthread
    

    Se quiser integrar o frontend com o backend, basta modificar o endpoint da requisição fetch no arquivo "frontend/script.js".

1. *Segunda Forma - Utilizando o docker*
    Se quiser utilizar o docker, apenas utilize o seguinte comando dentro do diretório do projeto

     
    docker compose up -d
    
---

## Conclusão
Este projeto foi desenvolvido como parte de um trabalho acadêmico da disciplina de Programação Paralela e Distribuída. Contribuições são muito bem-vindas!