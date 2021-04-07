#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

/* ------------ VARIABLES ------------- */
typedef struct
{
    double tIni;
    double tFin;
    long long sumParcial;
} datos;

typedef struct
{
    int id;
    int numDiv;
    long long div;
    double tpoCalculo;
} divisorProceso;
/* ------------------------------------ */

/* ------------------- PROTOTIPOS -------------------- */
long long pedirNumero();
long long acumDivUno(long long, int);
long long acumDivVarios(int, long long, long long, long long, MPI_Datatype *, int);
MPI_Datatype getMPI_Struct(divisorProceso *);
/* --------------------------------------------------- */

int main(int argc, char **argv)
{
    int id, nprocs;
    long long numEntrada;
    int defectivo; //Boolean: 1 o 0.
    long long temp;
    datos tiempo;
    divisorProceso divisorParcial;
    MPI_Datatype structDivisor;
    long long arrayDatos[3];
    long long ullIntervalo = 0L;
    long long ullTotal = 0L;
    long long acumDiv = 0L;
    MPI_Status status;
    int TAG = 50;
    int TAG1 = 51;
    int i;
    int x;
    int procesosFinalizados = 0;

    //Vectores usados para la salida por pantalla y para almacenar informacion de los procesos en el proceso 0
    double *tpoCalculo;
    int *numDivisores;
    long long *divParciales;

    long long resto = 0;
    /*Tags*/

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    structDivisor = getMPI_Struct(&divisorParcial);

    // ID 0 recibe el numero por teclado o linea de entrada
    if (id == 0)
    {
        if (argc >= 2)
        {
            numEntrada = atoll(argv[1]);
            if (numEntrada <= 0)
            {
                printf("\nERROR: El numero introducido debe ser > 0.\n");
                numEntrada = pedirNumero();
            }
        }
        else
        {
            numEntrada = pedirNumero();
        }
        if (nprocs == 1) // Solo hay un proceso
        {
            //Si solo hay un proceso llamamos a una variante que consiste en que el proc 0 haga todo el trabajo

            tiempo.tIni = MPI_Wtime();

            temp = acumDivUno(numEntrada, 0);

            tiempo.tFin = MPI_Wtime();

            //En funcion del resultado sabemos el tipo de numero
            if (temp == 0)
            {
                printf("El numero %lld es PERFECTO.\n", numEntrada);
            }
            else if (temp > 0)
            {
                printf("El numero %lld NO es PERFECTO, es EXCESIVO (%lld)\n", numEntrada, temp);
            }
            else // temp < 0
            {
                printf("El numero %lld NO es PERFECTO, es DEFECTIVO (%lld)\n", numEntrada, temp);
            }
            printf("Ejecucion: %lf\n", tiempo.tFin - tiempo.tIni);

            MPI_Finalize();

            return 0;
        }
        else // Si hay mas de un proceso
        {
            //Reservamos los vectores en funcion del numero de procesos que vaya haber quitando el proc 0
            tpoCalculo = (double *) malloc((nprocs - 1) * sizeof(double));
            numDivisores = (int *) malloc((nprocs - 1) * sizeof(int));
            divParciales = (long long *) malloc((nprocs - 1) * sizeof(long long));

            for (x = 0; x < nprocs - 1; x++)
            {
                divParciales[x] = 0;
                numDivisores[x] = 0;
                tpoCalculo[x] = 0;
            }

            /*Calculamos los intervalos del numero dividido entre dos, puesto que es imposible que un numero x
             *tenga un divisor mas grande que su mitad*/
            ullIntervalo = (numEntrada / 2) / (nprocs - 1);

            //Calcular si el numero tiene un resto para repartirlo entre los procesos
            resto = (numEntrada / 2) % (nprocs - 1);
        }

        arrayDatos[0] = ullIntervalo; //Intervalos calculados
        arrayDatos[1] = numEntrada;   //Numero que estamos calculando si es perfecto
        arrayDatos[2] = resto;        //Resto de la division de los intervalos
    }

    // Tiempo de inicio de la operacion
    tiempo.tIni = MPI_Wtime();

    MPI_Bcast(&arrayDatos, 3, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    if (id != 0)
    {
        // Función que devuelve el acumulador de cada hilo, y le pasa por referencia el struct del divisor
        acumDivVarios(id, arrayDatos[0], arrayDatos[1], arrayDatos[2], &structDivisor, nprocs);
    }
    else
    {
        while (procesosFinalizados < nprocs - 1)
        {
            MPI_Recv((void *)&divisorParcial, 1, structDivisor, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, &status);

            if (divisorParcial.div > 0)
            {
                acumDiv += divisorParcial.div;

                // Calculamos un vector con los divisores parciales de cada proceso
                divParciales[divisorParcial.id - 1] += divisorParcial.div;
                printf("DIV:%4d, DIV RECV:%20lld, DIV ACU:%4d, SUMA ACU: %20lld (%15lld)\n",
                       divisorParcial.id, divisorParcial.div, divisorParcial.numDiv, acumDiv, numEntrada - acumDiv);
            }
            else
            {
                procesosFinalizados++;
                printf("FIN:%4d, SUMA RECV:%20lld, SUMA ACU:%4d, SUMA ACU OK\n", divisorParcial.id, divParciales[divisorParcial.id - 1], divisorParcial.numDiv);

                tpoCalculo[divisorParcial.id - 1] = divisorParcial.tpoCalculo;
                numDivisores[divisorParcial.id - 1] = divisorParcial.numDiv;
            }
        }

        MPI_Recv(&ullTotal, 1, MPI_LONG_LONG, nprocs - 1, TAG1, MPI_COMM_WORLD, &status);
    }

    // Tiempo de fin de la operacion
    tiempo.tFin = MPI_Wtime();

    if (id == 0)
    {
        //Si el numero no coincide, significa que esta mal calculado
        if (ullTotal != acumDiv)
        {
            printf("\n\nSUMA TOTAL ERROR: calculada %lld recibida %lld O_o\n\n", acumDiv, ullTotal);
        }
        else
        {
            printf("\n\nSUMA TOTAL OK: calculada %lld recibida %lld\n\n", acumDiv, ullTotal);
        }

        i = 0;

        //TABLA
        printf("\nProceso | Nº Divisores   | Suma             | Tpo calculo\n");
        printf("--------------------------------------------------------------------\n");
        for (x = 1; x < nprocs; x++)
        {
            i += numDivisores[x - 1];
            printf("%7d | %14d | %16lld | %f\n", x, numDivisores[x - 1], divParciales[x - 1], tpoCalculo[x - 1]);
        }
        printf("--------------------------------------------------------------------\n");
        printf(" TOTAL  | %14d | %16lld\n\n", i, acumDiv);

        temp = numEntrada - ullTotal;

        if (temp == 0)
        {
            printf("El numero %lld es PERFECTO.\n", numEntrada);
        }
        else if (temp < 0)
        {
            printf("El numero %lld NO es PERFECTO, es EXCESIVO (%lld)\n", numEntrada, temp * -1);
        }
        else // temp > 0
        {
            printf("El numero %lld NO es PERFECTO, es DEFECTIVO (%lld)\n", numEntrada, temp);
        }
        printf("Numero de Procesos: %d\n", nprocs);
        printf("Tiempo procesamiento: %lf\n", tiempo.tFin - tiempo.tIni);
    }

    MPI_Finalize();

    return 0;
}

long long pedirNumero()
{
    long long numEntrada;
    do
    {
        printf("Introduce el numero: ");
        fflush(stdout);
        scanf("%lld", &numEntrada);
        if (numEntrada <= 0)
        {
            printf("\nERROR: El numero introducido debe ser > 0.\n");
        }
    } while (numEntrada <= 0);

    return numEntrada;
}

long long acumDivVarios(int id, long long ullIntervalo, long long nFinal, long long resto, MPI_Datatype *structDivisor, int nprocs)
{
    long long iInicio, iFinal;
    long long acumDivRecibido = 0L;
    long long divisorExtra;

    MPI_Status status;
    divisorProceso divisor;

    //Etiqueta: Identificador conversacion
    int TAG = 50;
    int TAG1 = 51;

    //Calculamos el intervalo para cada hilo
    iInicio = (id - 1) * ullIntervalo + 1;
    iFinal = (id - 1) * ullIntervalo + ullIntervalo;

    //Devuelve la resta del Acumulador de divisores con el numero pasado por parametro.
    long long l;
    long long acumDiv = 0L; // Acumulacion de la suma de divisores
    int i = 0;

    //Se rellena el struct que se usa para enviar los datos
    divisor.id = id;
    divisor.tpoCalculo = 0; // Se rellena en el momento en el que se han calculado todos los divisores
    divisor.numDiv = 0;

    double tIni = MPI_Wtime();
    double tFin;

    //Calculo sumatorio divisores en un intervalo
    for (l = iInicio; l <= iFinal; l++)
    {
        if (nFinal % l == 0 && nFinal - l != 0)
        {
            acumDiv += l;
            i++;

            divisor.div = l;
            divisor.numDiv = i;

            MPI_Send((void *)&divisor, 1, *structDivisor, 0, TAG, MPI_COMM_WORLD);
        }
    }

    // Reduce a la mitad el procesamiento, el intervalo reparte el resto a otros procesadores
    if (id <= resto)
    {
        //Se coge el numero de entrada dividido a la mitad, y se le resta el resto menos el identificador del proceso
        divisorExtra = ((nFinal / 2) - (resto - id));

        /*Ej: 28/3 (procesos) -> resto = 1
        divisorExtra = (28/2) - 1 - id

        Vemos que si el id es 1, que es el unico que entraria en el if(id <= resto), el divisorExtra = 14, que efectivamente
        seria el divisor que tendria que calcular extra el proc 1*/

        if ((nFinal % divisorExtra) == 0)
        {
            acumDiv += divisorExtra;
            i++; // Ha encontrado otro divisor

            divisor.div = divisorExtra;
            divisor.numDiv = i;

            MPI_Send((void *)&divisor, 1, *structDivisor, 0, TAG, MPI_COMM_WORLD);
        }
    }

    tFin = MPI_Wtime();

    if (id != 1)
    {
        MPI_Recv((void *)&acumDivRecibido, 1, MPI_LONG_LONG, id - 1, TAG1, MPI_COMM_WORLD, &status);
    }

    acumDiv += acumDivRecibido;

    if (id != nprocs - 1)
    {
        //Enviamos al proceso id+1 (id = nuestra id) nuestro acumDiv
        MPI_Send((void *)&acumDiv, 1, MPI_LONG_LONG, id + 1, TAG1, MPI_COMM_WORLD);
    }

    else
    {
        //Si el proceso es el ultimo
        MPI_Send((void *)&acumDiv, 1, MPI_LONG_LONG, 0, TAG1, MPI_COMM_WORLD);
    }

    //Enviamos un mensaje con div -1 indicando que el proceso ha terminado de calcular divisores
    divisor.div = -1;
    // Guardamos el tiempo de calculo de cada proceso en el struct
    divisor.tpoCalculo = tFin - tIni;
    MPI_Send((void *)&divisor, 1, *structDivisor, 0, TAG, MPI_COMM_WORLD);

    return acumDiv;
}

long long acumDivUno(long long iFinal, int id)
{
    //Devuelve la resta del Acumulador de divisores con el numero pasado por parametro.
    long long l;
    long long acumDiv = 0L; // Acumulacion de la suma de divisores
    int i = 0;
    //Calculo numero perfecto
    for (l = 1; l <= (iFinal / 2); l++)
    {
        if (iFinal % l == 0)
        {
            acumDiv += l;
            i++;
            printf("DIV:%4d, DIV RECV:%20lld, DIV ACU:%4d, SUMA ACU: %20lld (%15lld)\n", id, l, i, acumDiv, iFinal - acumDiv);
        }
    }
    return (acumDiv - iFinal);
}

MPI_Datatype getMPI_Struct(divisorProceso *dProceso)
{
    MPI_Datatype structDivisor;
    int longitudes[4];
    MPI_Aint desplaz[4];
    MPI_Aint direcc[5];
    MPI_Datatype tipos[4];

    tipos[0] = MPI_INT;
    tipos[1] = MPI_INT;
    tipos[2] = MPI_LONG_LONG;
    tipos[3] = MPI_DOUBLE;

    /* ESPECIFICAMOS EL NUMERO DE ELEMENTOS DE CADA TIPO */
    longitudes[0] = 1; // Un int
    longitudes[1] = 1; // Un int
    longitudes[2] = 1; // Un long long
    longitudes[3] = 1; // Un double

    /* CALCULAMOS LOS DESPLAZAMIENTOS DE LOS MIEMBROS DE LA ESTRUCTURA RELATIVOS AL COMIENZO DE LA MISMA */
    MPI_Get_address(dProceso, &direcc[0]);
    MPI_Get_address(&(dProceso->id), &direcc[1]);
    MPI_Get_address(&(dProceso->numDiv), &direcc[2]);
    MPI_Get_address(&(dProceso->div), &direcc[3]);
    MPI_Get_address(&(dProceso->tpoCalculo), &direcc[4]);

    desplaz[0] = direcc[1] - direcc[0];
    desplaz[1] = direcc[2] - direcc[0];
    desplaz[2] = direcc[3] - direcc[0];
    desplaz[3] = direcc[4] - direcc[0];

    /* CREACION DE TIPO DE DATOS DERIVADOS */
    MPI_Type_create_struct(4, longitudes, desplaz, tipos, &structDivisor);
    MPI_Type_commit(&structDivisor);

    return structDivisor;
}
