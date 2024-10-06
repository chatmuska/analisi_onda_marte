//
//  main.c
//  pro_entregar
//
//  Created by m-19 on 10/05/24.
//


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <math.h>
#include <fftw3.h>
#include <float.h>
#define PI 3.141592653589793


// Función que evalúa si una ventana es apta para estudio
bool es_ventana_apta(double amplitud_max, double tasa_cambio_amplitud, double entropia, double curtosis, double autocorrelacion) {
    // Define los umbrales para cada parámetro
    double umbral_amplitud_max = 0.5;          // Ajusta según tu caso
    double umbral_tasa_cambio_amplitud = 0.05; // Tasa baja indica menos ruido
    double umbral_entropia_max = 2.0;          // Baja entropía = señal ordenada
    double umbral_curtosis_min = 3.0;          // Curtosis alta indica eventos importantes
    double umbral_autocorrelacion_min = 0.5;   // Autocorrelación alta indica patrones repetitivos

    // Verifica si la ventana cumple con los criterios
    if (amplitud_max > umbral_amplitud_max &&
        tasa_cambio_amplitud < umbral_tasa_cambio_amplitud &&
        entropia < umbral_entropia_max &&
        curtosis > umbral_curtosis_min &&
        autocorrelacion > umbral_autocorrelacion_min) {
        return true; // La ventana es apta para un estudio más detallado
    }

    return false; // La ventana no es apta
}

// Prototipos de las funciones auxiliares
double calcular_amplitud_max(double *signal, int length);
double calcular_tasa_cambio_amplitud(double *signal, int length);
double calcular_entropia(double *signal, int length);
double calcular_curtosis(double *signal, int length);
double calcular_autocorrelacion(double *signal, int length, int lag);
void filtro_kalman(double *input, double *output, int length);

// Función para clasificar mini ondas sísmicas
void clasificar_mini_onda_sismica(double *signal, double dominant_freq, double ancho_banda, double *espectro_frecuencias, int num_frecuencias, double frecuencia_muestreo, int duracion_evento_minima, int ventana_analisis) {
    // Cálculo de umbrales fijos
    double umbral_amplitud_base = 5 * ancho_banda;
    int indice_freq_dominante = (int)(dominant_freq * num_frecuencias / (frecuencia_muestreo / 2));

    // Definir límites de la ventana de análisis
    int inicio_ventana = indice_freq_dominante - ventana_analisis / 2;
    int fin_ventana = indice_freq_dominante + ventana_analisis / 2;

    // Asegurar que los índices de ventana no salgan de rango
    if (inicio_ventana < 0) inicio_ventana = 0;
    if (fin_ventana >= num_frecuencias) fin_ventana = num_frecuencias - 1;

    // Inicializar variables para cálculos
    double amplitud_max = calcular_amplitud_max(signal + inicio_ventana, ventana_analisis);
    double tasa_cambio_amplitud = calcular_tasa_cambio_amplitud(signal + inicio_ventana, ventana_analisis);
    double entropia = calcular_entropia(signal + inicio_ventana, ventana_analisis);
    double curtosis = calcular_curtosis(signal + inicio_ventana, ventana_analisis);
    double autocorrelacion = calcular_autocorrelacion(signal + inicio_ventana, ventana_analisis, 10);
    
    // Imprimir valores calculados
    printf("amplitud_max: %lf\n", amplitud_max);
    printf("tasa_cambio_amplitud: %lf\n", tasa_cambio_amplitud);
    printf("Entropía: %lf\n", entropia);
    printf("Curtosis: %lf\n", curtosis);
    printf("autocorrelacion: %lf\n", autocorrelacion);

    // Ajuste dinámico de umbrales
    double umbral_amplitud = amplitud_max * 0.8;
    double umbral_tasa_cambio_impacto = tasa_cambio_amplitud * 0.5;
    double umbral_autocorrelacion = autocorrelacion * 0.7;
    double umbral_entropia = entropia * 0.6;
    double umbral_curtosis = curtosis * 0.8;

    // Detección y clasificación de eventos
    int duracion_evento = 0;
    double max_val = 0.0;

    for (int i = inicio_ventana; i <= fin_ventana; i++) {
        if (signal[i] > umbral_amplitud_base) {
            duracion_evento++;
        }
        if (signal[i] > max_val) {
            max_val = signal[i];
        }
    }

    // Aplicación de filtro de Kalman
    double *kalman_output = (double *)malloc(ventana_analisis * sizeof(double));
    if (kalman_output == NULL) {
        printf("Error de asignación de memoria para el filtro de Kalman.\n");
        return; // Salir si no se pudo asignar memoria
    }

    filtro_kalman(signal + inicio_ventana, kalman_output, ventana_analisis);

    // Análisis del espectro de frecuencias
    double promedio_potencia = 0.0;
    for (int i = 0; i < num_frecuencias; i++) {
        promedio_potencia += espectro_frecuencias[i];
    }
    promedio_potencia /= num_frecuencias;

    if (promedio_potencia > 50) {
        printf("Posible perturbación por ruido fuerte.\n");
    }

    free(kalman_output); // Liberar memoria asignada para el filtro de Kalman
}


// Función para aplicar un filtro de paso bajo simple
void filtro_paso_bajo(double *signal, double *output, int length, double cutoff) {
    double alpha = cutoff / (cutoff + 1.0); // Factor de suavizado
    output[0] = signal[0]; // Inicializar la salida

    for (int i = 1; i < length; i++) {
        output[i] = alpha * signal[i] + (1.0 - alpha) * output[i - 1]; // Filtro recursivo
    }
}

// Función para ajustar umbrales dinámicamente porque me da anchos de bandas enormes
void ajustar_umbrales(double *signal, int length, double *amplitud_threshold, double *amplitud_rate_threshold) {
    double max_amplitud = 0.0;
    double max_rate_of_change = 0.0;

    for (int i = 1; i < length; i++) {
        double amplitud = fabs(signal[i]);
        if (amplitud > max_amplitud) {
            max_amplitud = amplitud;
        }

        // Calcular tasa de cambio de amplitud
        double rate_of_change = fabs(signal[i] - signal[i - 1]);
        if (rate_of_change > max_rate_of_change) {
            max_rate_of_change = rate_of_change;
        }
    }

    // Ajustar umbrales al 50% del valor máximo detectado
    *amplitud_threshold = 0.5 * max_amplitud;
    *amplitud_rate_threshold = 0.5 * max_rate_of_change;
}


void calcular_espectro_real(fftw_complex *espectro, double *espectro_real, int num_frecuencias) {
    for (int i = 0; i < num_frecuencias; i++) {
        double parte_real = espectro[i][0];  // Parte real
        double parte_imaginaria = espectro[i][1];  // Parte imaginaria
        // Calcular la magnitud
        espectro_real[i] = sqrt(parte_real * parte_real + parte_imaginaria * parte_imaginaria);
    }
}


double calcular_autocorrelacion(double *signal, int ventana_analisis, int max_desplazamiento) {
    // ... (código para calcular la media de la señal)
    // Calcular la media
        double suma = 0.0;
        for (int i = 0; i < ventana_analisis; i++) {
            suma += signal[i];
        }
        double media = suma / ventana_analisis;
    
    double autocorrelacion_max = 0.0;
    for (int desplazamiento = 1; desplazamiento <= max_desplazamiento; desplazamiento++) {
        double suma_producto = 0.0;
        for (int i = 0; i < ventana_analisis - desplazamiento; i++) {
            suma_producto += (signal[i] - media) * (signal[i + desplazamiento] - media);
        }
        double correlacion = suma_producto / (ventana_analisis - desplazamiento);
        if (fabs(correlacion) > autocorrelacion_max) {
            autocorrelacion_max = fabs(correlacion);
        }
    }
    return autocorrelacion_max;
}
// esta funcion es para clasificar_onda_ruido 01
double calcular_curtosis(double *signal, int LUX) {
    double media = 0.0, varianza = 0.0, curtosis = 0.0;
    
    // Calcular la media
    for (int i = 0; i < LUX; i++) {
        media += signal[i];
    }
    media /= LUX;

    // Calcular la varianza y curtosis
    for (int i = 0; i < LUX; i++) {
        double diff = signal[i] - media;
        varianza += diff * diff;
        curtosis += diff * diff * diff * diff;
    }
    
    varianza /= LUX;
    curtosis /= LUX;

    // Normalizar la curtosis
    if (varianza > 0) {
        curtosis = curtosis / (varianza * varianza) - 3.0;
    }

    return curtosis;
}
//esta funcion es para clasificar_onda_ruido 02
double calcular_entropia(double *signal, int LUX) {
    double entropia = 0.0;
    double total = 0.0;
    
    // Calcular la suma total de los valores absolutos de la señal
    for (int i = 0; i < LUX; i++) {
        total += fabs(signal[i]);
    }
    
    // Calcular la entropía
    for (int i = 0; i < LUX; i++) {
        double p = fabs(signal[i]) / total;
        if (p > 0) {
            entropia -= p * log(p);
        }
    }
    
    return entropia;
}

//esta funcion es para clasificar_onda_ruido 04
double calcular_tasa_cambio_amplitud(double *signal, int LUX) {
    double max_cambio = 0.0;
    for (int i = 1; i < LUX; i++) {
        double cambio = fabs(signal[i] - signal[i - 1]);
        if (cambio > max_cambio) {
            max_cambio = cambio;
        }
    }
    return max_cambio;
}

//esta funcion es para clasificar_onda_ruido 03
double calcular_amplitud_max(double *signal, int LUX) {
    double max_amplitud = signal[0];
    for (int i = 1; i < LUX; i++) {
        if (signal[i] > max_amplitud) {
            max_amplitud = signal[i];
        }
    }
    return max_amplitud;
}

void filtro_kalman(double *signal, double *output, int LUX) {
    double x_est = 0.0, p_est = 1.0;  // Estado estimado y varianza
    double Q = 0.001, R = 1.0;        // Ruido de proceso y ruido de medición

    for (int i = 0; i < LUX; i++) {
        // Predicción
        double x_pred = x_est;
        double p_pred = p_est + Q;
        
        // Actualización
        double K = p_pred / (p_pred + R);  // Ganancia de Kalman
        x_est = x_pred + K * (signal[i] - x_pred);
        p_est = (1 - K) * p_pred;
        
        // Guardar el valor filtrado
        output[i] = x_est;
    }
}
// Función para calcular el ancho de banda 06
double calcular_ancho_banda(double *espectro_real, double *frecuencias, int num_frecuencias) {
    double suma_potencia = 0.0;
    double suma_frecuencia_ponderada = 0.0;

    // Calcular la potencia total y la suma de las frecuencias ponderadas por la potencia
    for (int i = 0; i < num_frecuencias; i++) {
        suma_potencia += espectro_real[i];
        suma_frecuencia_ponderada += frecuencias[i] * espectro_real[i];
    }

    // Umbral para espectros muy débiles
    double umbral = 1e-10;
    if (suma_potencia < umbral) {
        printf("Espectro muy débil o nulo. Considera ajustar los parámetros o verificar los datos.\n");
        return NAN;
    }

    // Calcular la frecuencia centroidal (centroide de frecuencia)
    double frecuencia_central = suma_frecuencia_ponderada / suma_potencia;

    double ancho_banda = 0.0;
    for (int i = 0; i < num_frecuencias; i++) {
        // Calcular la dispersión alrededor de la frecuencia central, ponderada por la potencia
        ancho_banda += espectro_real[i] * (frecuencias[i] - frecuencia_central) * (frecuencias[i] - frecuencia_central);
    }

    // Dividir por la potencia total y tomar la raíz cuadrada para obtener el ancho de banda
    return sqrt(ancho_banda / suma_potencia);
}


// Cálculo dinámico de SNR 09

// Función para calcular SNR
double calcular_SNR(double *signal, int length, double noise_threshold) {
    double signal_power = 0.0;
    double noise_power = 0.0;
    int signal_count = 0, noise_count = 0;

    for (int i = 0; i < length; i++) {
        // Si el valor de la señal excede el umbral, se considera señal; de lo contrario, ruido
        if (fabs(signal[i]) > noise_threshold) {
            signal_power += signal[i] * signal[i];
            signal_count++;
        } else {
            noise_power += signal[i] * signal[i];
            noise_count++;
        }
    }

    // Evitar divisiones por cero
    if (signal_count == 0 || noise_count == 0 || noise_power == 0) {
        return NAN; // Retorna 'NAN' si hay un problema
    }

    // Calcular el SNR en dB
    double snr = 10 * log10((signal_power / signal_count) / (noise_power / noise_count));
    return snr;
}




// Función para clasificar ondas de ruido
void clasificar_onda_ruido(double *signal, double dominant_freq, double ancho_banda, double *espectro_frecuencias, int num_frecuencias, double frecuencia_muestreo, int duracion_evento_minima, int ventana_analisis) {
    // Cálculo de umbrales fijos
    double umbral_amplitud_base = 5 * ancho_banda;
    int indice_freq_dominante = (int)(dominant_freq * num_frecuencias / (frecuencia_muestreo / 2));

    // Definir límites de la ventana de análisis
    int inicio_ventana = indice_freq_dominante - ventana_analisis / 2;
    int fin_ventana = indice_freq_dominante + ventana_analisis / 2;

    // Asegurar que los índices de ventana no salgan de rango
    if (inicio_ventana < 0) inicio_ventana = 0;
    if (fin_ventana >= num_frecuencias) fin_ventana = num_frecuencias - 1;

    // Inicializar variables para cálculos
    double amplitud_max = calcular_amplitud_max(signal + inicio_ventana, ventana_analisis);
    double tasa_cambio_amplitud = calcular_tasa_cambio_amplitud(signal + inicio_ventana, ventana_analisis);
    double entropia = calcular_entropia(signal + inicio_ventana, ventana_analisis);
    double curtosis = calcular_curtosis(signal + inicio_ventana, ventana_analisis);
    double autocorrelacion = calcular_autocorrelacion(signal + inicio_ventana, ventana_analisis, 10);
    
    // Imprimir valores calculados
    printf("amplitud_max: %lf\n", amplitud_max);
    printf("tasa_cambio_amplitud: %lf\n", tasa_cambio_amplitud);
    printf("Entropía: %lf\n", entropia);
    printf("Curtosis: %lf\n", curtosis);
    printf("autocorrelacion: %lf\n", autocorrelacion);

    // Ajuste dinámico de umbrales
    double umbral_amplitud = amplitud_max * 0.8;
    double umbral_tasa_cambio_impacto = tasa_cambio_amplitud * 0.5;
    double umbral_autocorrelacion = autocorrelacion * 0.7;
    double umbral_entropia = entropia * 0.6;
    double umbral_curtosis = curtosis * 0.8;

    // Detección y clasificación de eventos
    int duracion_evento = 0;
    double max_val = 0.0;

    for (int i = inicio_ventana; i <= fin_ventana; i++) {
        if (signal[i] > umbral_amplitud_base) {
            duracion_evento++;
        }
        if (signal[i] > max_val) {
            max_val = signal[i];
        }
    }

   

    // Aplicación de filtro de Kalman
    double *kalman_output = (double *)malloc(ventana_analisis * sizeof(double));
    if (kalman_output == NULL) {
        printf("Error de asignación de memoria para el filtro de Kalman.\n");
        return; // Salir si no se pudo asignar memoria
    }

    filtro_kalman(signal + inicio_ventana, kalman_output, ventana_analisis);

    // Análisis del espectro de frecuencias
    double promedio_potencia = 0.0;
    for (int i = 0; i < num_frecuencias; i++) {
        promedio_potencia += espectro_frecuencias[i];
    }
    promedio_potencia /= num_frecuencias;

    if (promedio_potencia > 50) {
        printf("Posible perturbación por ruido fuerte.\n");
    }

    free(kalman_output); // Liberar memoria asignada para el filtro de Kalman
}



// Función que clasifica la onda según la frecuencia dominante 07
void clasificar_onda(double dominant_freq) {
    if (dominant_freq < 0.01) {
        printf("Frecuencia demasiado baja para una clasificación confiable.\n");
    }
    else if (dominant_freq >= 0.01 && dominant_freq <= 0.05) {
        printf("Posible onda sísmica marciana de muy baja frecuencia (0.01 - 0.05 Hz)\n");
    }
    else if (dominant_freq > 0.05 && dominant_freq <= 0.1) {
        printf("Posible onda sísmica marciana de baja frecuencia (0.05 - 0.1 Hz)\n");
    }
    else if (dominant_freq > 0.1 && dominant_freq <= 0.5) {
        printf("Posible onda sísmica marciana de frecuencia baja a moderada (0.1 - 0.5 Hz)\n");
    }
    else if (dominant_freq > 0.5 && dominant_freq <= 1.0) {
        printf("Posible onda sísmica marciana de frecuencia moderada (0.5 - 1.0 Hz)\n");
    }
    else if (dominant_freq > 1.0 && dominant_freq <= 2.0) {
        printf("Posible ruido impulsivo o vibraciones de origen no sísmico (1.0 - 2.0 Hz)\n");
    }
    else if (dominant_freq > 2.0 && dominant_freq <= 5.0) {
        printf("Posible ruido ambiental o ruido sísmico menor (2.0 - 5.0 Hz)\n");
    }
    else if (dominant_freq > 5.0 && dominant_freq <= 20.0) {
        printf("Posible ruido de alta frecuencia o interferencias (5.0 - 20.0 Hz)\n");
    }
    else {
        printf("Frecuencia dominante fuera de los rangos esperados.\n");
    }
}


// Función para calcular la frecuencia dominante usando FFT 05
double calcular_frecuencia_dominante(double *data, int LUX, double sampling_rate) {
    fftw_complex *out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * (LUX/ 2 + 1));
    fftw_plan plan = fftw_plan_dft_r2c_1d(LUX, data, out, FFTW_ESTIMATE);

    // Ejecutar la FFT
    fftw_execute(plan);

    double max_magnitude = 0.0;
    int dominant_index = 0;

    // Calcular magnitudes de las frecuencias y encontrar la máxima
    for (int i = 0; i < LUX / 2 + 1; i++) {
        double magnitude = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
        if (magnitude > max_magnitude) {
            max_magnitude = magnitude;
            dominant_index = i;
        }
    }

    // La frecuencia correspondiente al índice dominante
    double dominant_freq = (double)dominant_index * sampling_rate / LUX;

    // Liberar recursos
    fftw_destroy_plan(plan);
    fftw_free(out);

    return dominant_freq;
}



// Funciones auxiliares
double calcular_SNR(double *signal, int length, double noise_threshold);
void ajustar_umbrales(double *signal, int length, double *amplitud_threshold, double *amplitud_rate_threshold);
void filtro_paso_bajo(double *signal, double *output, int length, double cutoff);

// Aquí también debería incluirse la función 'calcular_frecuencia_dominante', 'calcular_espectro_real', etc.

// Otras inclusiones y definiciones necesarias...

// Define tus funciones previamente aquí, incluyendo las funciones de análisis.

void procesar_archivo_csv(const char *archivo) {
    printf("Intentando abrir el archivo: %s\n", archivo);
    FILE *fp = fopen(archivo, "r");
    if (!fp) {
        perror("Error al abrir el archivo");
        return;
    }

    printf("Archivo %s abierto correctamente.\n", archivo);
    char linea[100];
    if (fgets(linea, sizeof(linea), fp) == NULL) {
        fprintf(stderr, "Error al leer los encabezados\n");
        fclose(fp);
        return;
    }
    printf("Encabezado descartado: %s\n", linea);
    
    // Lee los valores del archivo
    double *data = NULL;  // Inicializamos como NULL para usar realloc más tarde
    int LUX = 0;          // Contador de muestras
    double value;

    while (fscanf(fp, "%*[^,],%*[^,],%lf", &value) == 1) {
        data = realloc(data, (LUX + 1) * sizeof(double));  // Aumentar el tamaño del arreglo
        if (data == NULL) {
            fprintf(stderr, "Error al asignar memoria\n");
            fclose(fp);
            return;
        }
        data[LUX] = value;  // Almacena el valor
        LUX++;
    }
    printf("Muestras %d\n", LUX);
    fclose(fp);

    // Verifica que LUX sea válido
    if (LUX <= 0) {
        fprintf(stderr, "Error: No se leyeron datos válidos.\n");
        free(data);  // Libera la memoria si no hay datos
        return;
    }

    // **1. Aplicar filtro de paso bajo antes del análisis**
    double *filtered_data = (double *)malloc(LUX * sizeof(double));
    filtro_paso_bajo(data, filtered_data, LUX, 0.1);  // Cutoff de 0.1 (ajusta según sea necesario)
    printf("Filtro de paso bajo aplicado.\n");

    // **2. Ajustar umbrales dinámicos de amplitud y tasa de cambio de amplitud**
    double amplitud_threshold = 0.0;
    double amplitud_rate_threshold = 0.0;
    ajustar_umbrales(filtered_data, LUX, &amplitud_threshold, &amplitud_rate_threshold);
    printf("Umbrales ajustados: Amplitud: %f, Tasa de cambio de amplitud: %f\n", amplitud_threshold, amplitud_rate_threshold);

    // **3. Definir parámetros para el análisis de mini ventanas**
    int ventana_analisis = 1024; // Tamaño de cada mini ventana
    int max_desplazamiento = 10;  // Desplazamiento máximo para autocorrelación

    

    // Calcular la frecuencia dominante
    double sampling_rate = 1000.0;  // Ejemplo: 1000 Hz adaptado a Marte
    double dominant_freq = calcular_frecuencia_dominante(filtered_data, LUX, sampling_rate);
    printf("Frecuencia dominante: %f Hz\n", dominant_freq);

    // **4. Calcular SNR para la señal filtrada**
    double noise_threshold = amplitud_threshold * 0.1;  // Establece un umbral de ruido basado en el umbral de amplitud
    double snr = calcular_SNR(filtered_data, LUX, noise_threshold);
    if (isnan(snr)) {
        printf("SNR no pudo calcularse. Verifica el umbral de ruido.\n");
    } else {
        printf("SNR calculado: %f dB\n", snr);
    }

    // Generar el espectro de frecuencias usando FFT
    fftw_complex *espectro = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * (LUX / 2 + 1));
    fftw_plan plan = fftw_plan_dft_r2c_1d(LUX, filtered_data, espectro, FFTW_ESTIMATE);
    fftw_execute(plan);
    
    // Calcular el espectro real
    double *espectro_real = (double *)malloc((LUX / 2 + 1) * sizeof(double));
    calcular_espectro_real(espectro, espectro_real, LUX / 2 + 1);

    // Crear el array de frecuencias
    double *frecuencias = (double *)malloc((LUX / 2 + 1) * sizeof(double));
    for (int i = 0; i < LUX / 2 + 1; i++) {
        frecuencias[i] = (double)i * (sampling_rate / LUX);
    }

    // Calcular el ancho de banda usando el espectro real
    double ancho_banda = calcular_ancho_banda(espectro_real, frecuencias, LUX / 2 + 1);
    printf("Ancho de banda calculado: %lf\n", ancho_banda);
    
    
    // Si magnitudes se refiere al espectro real, puedes usarlo directamente
    double *magnitudes = espectro_real;

    // Usar las magnitudes en la función clasificar_onda_ruido
    clasificar_onda_ruido(filtered_data, dominant_freq, ancho_banda, magnitudes, LUX / 2 + 1, sampling_rate, 50, 20);
    
    // Variables para almacenar resultados
    int ventanas_aptas = 0;  // Contador de ventanas aptas
    // Aplicar análisis de mini ventanas
    for (int i = 0; i < LUX; i += ventana_analisis) {
        int ventana_length = fmin(ventana_analisis, LUX - i); // Asegúrate de que no te salgas del arreglo

        // Crear una ventana temporal
        double *ventana = (double *)malloc(ventana_length * sizeof(double));
        memcpy(ventana, filtered_data + i, ventana_length * sizeof(double));

        // Llamar a funciones de análisis sobre la ventana
        double amplitud_max = calcular_amplitud_max(ventana, ventana_length);
        double tasa_cambio_amplitud = calcular_tasa_cambio_amplitud(ventana, ventana_length);
        double entropia = calcular_entropia(ventana, ventana_length);
        double curtosis = calcular_curtosis(ventana, ventana_length);
        double autocorrelacion = calcular_autocorrelacion(ventana, ventana_length, max_desplazamiento);
        
        // Imprimir valores calculados para cada ventana
        printf("Ventana %d:\n", i / ventana_analisis);
        printf("  Amplitud Max: %lf\n", amplitud_max);
        printf("  Tasa de Cambio de Amplitud: %lf\n", tasa_cambio_amplitud);
        printf("  Entropía: %lf\n", entropia);
        printf("  Curtosis: %lf\n", curtosis);
        printf("  Autocorrelación: %lf\n", autocorrelacion);
         
        // Evaluar si la ventana es apta
               // bool apta = es_ventana_apta(amplitud_max, tasa_cambio_amplitud, entropia, curtosis, autocorrelacion);
                //if (apta) {
                  //  printf("Ventana %d es apta para estudio más detallado.\n", i / ventana_analisis);
                    //ventanas_aptas++;  // Aumentar el contador de ventanas aptas
                //} else {
                  //  printf("Ventana %d no es apta.\n", i / ventana_analisis);
                //}        // Liberar la ventana
        free(ventana);
    }

    // Liberar la memoria correctamente
    free(espectro_real);  // Esto también libera 'magnitudes', ya que es lo mismo
    free(frecuencias);
    fftw_destroy_plan(plan);
    fftw_free(espectro);
    free(data);
    free(filtered_data);  // Liberar también la señal filtrada
}


int main(void) {//00
    char carpeta[100];
    DIR *dir;
    struct dirent *ent;

    printf("Ingrese la ruta de la carpeta: ");
    scanf("%s", carpeta);
    dir = opendir(carpeta);
    if (dir == NULL) {
        perror("Error al abrir la carpeta");
        return 1;
    }

    while ((ent = readdir(dir)) != NULL) {
        if (strstr(ent->d_name, ".csv") != NULL) {
            char archivo[200];
            sprintf(archivo, "%s/%s", carpeta, ent->d_name);
            procesar_archivo_csv(archivo);
        }
    }

    closedir(dir);
    return 0;
}
