FROM gcc:14-bookworm

WORKDIR /app

COPY . .

RUN g++ -fopenmp -o threshold_adaptive main.cpp -pthread

EXPOSE 8080

CMD ["./threshold_adaptive"] 