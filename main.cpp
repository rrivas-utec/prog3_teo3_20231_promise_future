#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <future>
#include <cmath>
using namespace std;

//-------------------------------------------------

mutex mtx; // Definido en el contexto Global, Local (dentro de una funcion, dentro de una clase como atributo)

struct Message {
    string text;
    bool sent{};
};

void transmisor(Message& msg) {
    this_thread::sleep_for(chrono::seconds(3));
    unique_lock ul(mtx);
    msg.sent = true;
    msg.text = "Enviado por Hilo 2";
    ul.unlock();
    while (msg.sent == true) {}     // Esperar que el mensaje haya sido recepcionado
    cout << "Mensaje exitosamente enviado";
}

void receptor(Message& msg) {
    while (msg.sent == false) {}    // Esperar el mensaje
    unique_lock ul(mtx);
    cout << "Mensaje recepcionado: " << msg.text << endl;
    this_thread::sleep_for(chrono::seconds(3));
    msg.sent = false;
}

void ejemplo_mensajeria_x_variable_compartida() {
    Message msg;
    thread t1(receptor, ref(msg));
    thread t2(transmisor, ref(msg));
    t1.join();
    t2.join();
}

//-------------------------------------------------

void producer(promise<Message> prm) {
    this_thread::sleep_for(chrono::seconds(3));
    Message msg("Enviado por Producer");
    prm.set_value(msg);
    lock_guard lg(mtx);
    cout << "Mensaje exitosamente enviado\n";
}

void consumer(future<Message> fut) {
    auto msg = fut.get();
    this_thread::sleep_for(chrono::seconds(3));
    lock_guard lg(mtx);
    cout << "Mensaje recepcionado: " << msg.text << endl;
}

void ejemplo_mensajeria_x_promise_future() {
    // Instanciar el promise, future y la conexion entre ambos
    promise<Message> prm;
    future<Message> fut = prm.get_future();

    // Crear y asignar tarea a los 2 hilos
    thread h_producer(producer, std::move(prm));
    thread h_consumer(consumer, std::move(fut));

    // Unir los 2 hilos
    h_producer.join();
    h_consumer.join();
}

//-------------------------------------------------

void sumar_ref(int a, int b, int& total) {
    total = a + b;
}

void sumar(int a, int b, int& total) {
    total = a + b;
}
int sumar(int a, int b) {
    return a + b;
}

void coseno(double radianes, double& result) {
    result = cos(radianes);
}

void ejemplo_comparar_threads_vs_asyncs() {
    // Ejemplo usando sumar
    int total = 0;
    thread t1(sumar_ref, 10, 20, ref(total));   // Los hilos no soportan sobre carga
    t1.join();
    cout << total << endl;

    total = 0;
    t1 = thread([&total]{ total = sumar(10, 20); });
    t1.join();
    cout << total << endl;

    auto wrapper_sum = [& total](auto a, auto b){ sumar(a, b, total); };
    t1 = thread(wrapper_sum, 10, 20);
    t1.join();
    cout << total << endl;

    // Ejemplo usando cosenos
    double radianes = 3.1416 / 4;

    double cs = 0;
    thread t2(coseno, radianes, ref(cs));
    t2.join();
    cout << cs << endl;



    // Usando async con coseno
    // Cuando existe sobrecarga de una funcion la solucion es realizar un wrapper utilizado lambdas

    auto fut_cs = async([radianes]{ return cos(radianes);});   // future<double>
    cout << fut_cs.get() << endl;

    auto wrapper_cos = [](auto rad){ return cos(rad);};
    auto fut_cs2 = async(wrapper_cos, radianes);
    cout << fut_cs2.get() << endl;

}


//-------------------------------------------------

int main() {
//    ejemplo_mensajeria_x_variable_compartida();
//    ejemplo_mensajeria_x_promise_future();
    ejemplo_comparar_threads_vs_asyncs();
    return 0;
}
