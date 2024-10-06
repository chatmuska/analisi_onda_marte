# analisi_onda_marte
This is a software that analyzes Martian waves and classifies them as possible seismic waves based on calculated characteristics.
I wrote this code in Xcode, but it is not programmed in Swift because I don’t know much of that language.
I wrote it in C programming because I know something about it, so I run it from the terminal. Let me explain:
[compiler gcc] [main prototype] [dash lowercase ho] [folder path pro_entregar.out] [this command is to use the library #include <fftw3.h>]
% gcc main.c -o pro_entregar.out -lfftw3  
% ./pro_entregar.out  
Enter the folder path: /Users/m-19/Desktop/nasa/pro_entregar/pro_entregar/xdatadd
I made a decision to use this file with the following location: space_apps_2024_seismic_detection/data/mars/training/data
because I have a very old Mac, and when processing lunar data, the computer tended to freeze.
"Well, let’s talk about the code"
This code analyzes seismic waves and takes the data from the .csv file.
Then, it analyzes the wave with various filters and global techniques to reduce noise.
It has adaptive thresholds because when it’s on the planet Mars, no one will configure it, so they adapt based on the data statistics.
Based on that, it analyzes the wave with two windows: one global that analyzes the entire frequency in general,
I also implemented a sliding window technique to analyze the wave in small windows, to make sure no event is missed.
"This part is missing from my code. I couldn’t complete it because I have commitments on Sunday, but I think it would take me a couple of days to complete it."
It’s the part where the global window and the sliding window compare their data and evaluate whether the wave is seismic or just noise. Sadly for me, I couldn’t finish it.
Conclusion
There’s something I want to say: I’m just a simple enthusiast, and I’ve never had so much fun or learned so much in a field that I had no idea about what I’ve done.





este es un software que analiza ondas marcianas y la clasifica en posible onda sísmica en base a características calculada
este codigo yo lo escribi en xcode pero no esta programado en switf porque no se mucho de ese idioma.
yo lo escribi en programacion.c que se algo por eso lo ejecuto desde la terminal le explico 
[conpilador gcc][prototipo main] [letra menos ho][ruta de carpeta pro_entregar.out][este comando es para utilizar la biblioteca #include <fftw3.h>] 
% gcc main.c -o pro_entregar.out -lfftw3
% ./pro_entregar.out                    
Ingrese la ruta de la carpeta: /Users/m-19/Desktop/nasa/pro_entregar/pro_entregar/xdatadd 
tome una decicion de utilizar este archivo con la siguiente localizacion: space_apps_2024_seismic_detection/data/mars/training/data
porque tengo una mac muy antigia y cuando procesaba datos lunares la computadora tendia a pausarse
"bueno hablemos del codigo" 
este codigo analiza ondas sismicas y toma los datos en el archivo.scv 
luego analiza la onda con distintos filtro y tecnicas globales para reducir el ruido
tiene humbrales adaptativos porque cuando este en el planeta marte nadie lo configurara asi que se adaptan segun las estadisticas de los datos 
en base a eso analizan la onda con dos ventanas una global analiza toda la frecuencia en general 
tambien inplemente una tecnica de ventanas corredisas para que tambien analizaran la onda en pequenas ventanas para no dejar pasar nimgun evento 
"esta parte si le falta a mi coigo no pude completarla porque yo tengo compromisos el domingo igual creo que me tomaria un par de dias completar eso "
es la parte a domde la ventana global y la corredisa conparan sus datos y evaluan si la onda en sismica o solo ruido.tristemente para mi lo pude hacer
conclusion
hay algo que si quiero decirle yo soy un simple fanatico y de verdad numca me habia divertido tanto y aprendido tanto en un campo que no tenia ni idea de lo que he echo 
