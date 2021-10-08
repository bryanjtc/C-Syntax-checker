#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int numVariables = 0;
struct var *vars = NULL;

struct linea
{
    char *texto;
    enum
    {
        NA,
        LECTURA,
        ESCRITURA,
        DECLARACION,
        ASIGNACION
    } tipo;
    int error;
};

struct var
{
    char *nombre;
    enum
    {
        NADA,
        ENTERO,
        REAL,
        CARACTER,
        CADENA
    } tipo;
};

int contarLineas()
{
    FILE *archivo;
    int count = 0;

    archivo = fopen("basico.txt", "r");
    while (!feof(archivo))
        if (getc(archivo) == '.')
            count++;
    fclose(archivo);
    return count;
}

void extraerLineas(struct linea *lineas, int numLineas)
{
    FILE *archivo;
    int i, j;
    char c;
    archivo = fopen("basico.txt", "r");
    for (i = 0; i < numLineas; i++)
    {
        j = 0;
        while ((c = getc(archivo)) != '.')
            j++;

        lineas[i].texto = malloc(j + 1);
        fseek(archivo, -(j + 1), SEEK_CUR);
        for (j = 0; (c = getc(archivo)) != '.'; j++)
            if (c == '\n')
                j--;
            else
                lineas[i].texto[j] = c;
        lineas[i].texto[j] = '\0';
        lineas[i].error = 0;
    }
    fclose(archivo);
}

int verificarDeclarada(char *nombre, int tipo)
{
    int i;
    for (i = 0; i < numVariables; i++)
        if (!strcmp(vars[i].nombre, nombre))
        {
            if (vars[i].tipo == tipo)
                return 1;
            else
                return -1;
        }
        else
            return 0;
}

struct var *agregarVar(struct var *vars, char *nombre, int tipo)
{
    vars = realloc(vars, (numVariables + 1) * sizeof(struct var));

    vars[numVariables].nombre = malloc(strlen(nombre) + 1);
    strcpy(vars[numVariables].nombre, nombre);
    vars[numVariables].tipo = tipo;
    numVariables++;
    return vars;
}

void declaracion(struct linea *line)
{
    int i = 0, j = 0, tipo;
    while (*(line->texto + i) == ' ')
        i++;
    while (isalnum(*(line->texto + i + j)))
        j++;

    if (!strncmp(line->texto + i, "real", j))
    {        
        i += j;
        tipo = REAL;
    }
    else if (!strncmp(line->texto + i, "entero", j))
    {
        i += j;
        tipo = ENTERO;
    }
    else if (!strncmp(line->texto + i, "cadena", j))
    {
        i += j;
        tipo = CADENA;
    }
    else if (j == 8 && !strncmp(line->texto + i, "caracter", j))
    {
        i += j;
        tipo = CARACTER;
    }
    else
    {
        line->error = 1;
        return;
    }

    while (*(line->texto + i) == ' ')
        i++;
    j = 0;
    if (!isalpha(*(line->texto + i)) && !islower(*(line->texto + i)))
    {
        line->error = 2;
        return;
    }
    while (*(line->texto + i + j) != '\0')
    {
        if (!isalnum(*(line->texto + i + j)))
        {
            line->error = 3;
            return;
        }
        j++;
    }
    vars = agregarVar(vars, (line->texto + i), tipo);
    return;
}

void asignacion(struct linea *line)
{
    int i = 0, j = 0;
    char *str, last = 'o';
    while (*(line->texto + i) == ' ')
        i++;
    while (isalnum(*(line->texto + i + j)))
        j++;
    str = malloc(j + 1);
    strncpy(str, (line->texto + i), j);
    str[j] = '\0';
    if (!verificarDeclarada(str, NADA))
    {
        line->error = 1;
        free(str);
        return;
    }
    i += j;
    free(str);
    while (*(line->texto + i) == ' ')
        i++;
    if (*(line->texto + i) != '=')
    {
        line->error = 2;
        free(str);
        return;
    }
    i++;
    while (*(line->texto + i) != '\0')
    {
        while (*(line->texto + i) == ' ')
            i++;
        if (!isalnum(*(line->texto + i)))
        {
            line->error = 3;
            free(str);
            return;
        }
        if(isdigit(*(line->texto + i)) && last == 'o')
            {
                while (isdigit(*(line->texto + i)))
                    i++;
                last = 'e';
            }
        else if (isalnum(*(line->texto + i)) && last == 'o')
        {
            j = 0;
            while (isalnum(*(line->texto + i + j)))
                j++;
            str = malloc(j + 1);
            strncpy(str, (line->texto + i), j);
            str[j] = '\0';
            if (!verificarDeclarada(str, NADA))
            {
                line->error = 1;
                free(str);
                return;
            }
            i += j;
            last = 'e';
        }
        else if(*(line->texto + i) == ' ')
            {
                while (*(line->texto + i) == ' ')
                i++;
                last = ' ';
            }
        else
        {
            line->error = 4;
            free(str);
            return;
        }
        while (*(line->texto + i) == ' ')
            i++;
        if(*(line->texto + i) == '\0')
        {
            free(str);
            return;
        }
        if (last == 'e')        
        {
            if (*(line->texto + i) != '+' && *(line->texto + i) != '-' && *(line->texto + i) != '\\' && *(line->texto + i) != '/' && *(line->texto + i) != '*')
            {               
                line->error = 4;
                free(str);
                return;
            } 
            else
            {
                last = 'o';
                i++;
            }
        }                   
    }
    free(str);
    return;
}

void lectura(struct linea *line)
{
    int i = 0, j = 0;
    char *str;
    
    while (*(line->texto + i) == ' ')
        i++;
    if (strncmp(line->texto + i, "Leer", 4))
    {
        line->error = 1;
        return;
    }
    i += 4;
    while (*(line->texto + i) == ' ')
        i++;
    if (*(line->texto + i) != '(')
    {
        line->error = 2;
        return;
    }
    i++;
    while (*(line->texto + i) == ' ')
        i++;
    while (isalnum(*(line->texto + i + j)))
        j++;
    str = malloc(j + 1);
    strncpy(str, (line->texto + i), j);
    str[j] = '\0';
    if (!verificarDeclarada(str, NA))
    {
        line->error = 3;
        free(str);
        return;
    }
    i += j;
    while (*(line->texto + i) == ' ')
        i++;
    if (*(line->texto + i) != ')')
    {
        line->error = 2;
        free(str);
        return;
    }
    free(str);
    return;
}

void escritura(struct linea *line)
{
    int i = 0, j = 0;
    char *str, last = ',';

    while (*(line->texto + i) == ' ')
        i++;
    if (strncmp(line->texto + i, "Escribir", 8))
    {
        line->error = 1;
        return;
    }
    i += 8;
    while (*(line->texto + i) == ' ')
        i++;
    if (*(line->texto + i) != '(')
    {
        line->error = 2;
        return;
    }
    i++;
    while (*(line->texto + i) == ' ')
        i++;

    while (*(line->texto + i) != '\0' && *(line->texto + i) != ')')
    {
        if (isalnum(*(line->texto + i)) && last == ',')
        {
            while (isalnum(*(line->texto + i + j)))
                j++;
            str = malloc(j + 1);
            strncpy(str, (line->texto + i), j);
            str[j] = '\0';
            if (!verificarDeclarada(str, NA))
            {
                line->error = 3;
                free(str);
                return;
            }
            i += j;
            agregarVar(vars, str, NA);
            free(str);
            last = 'e';
        }
        else if (*(line->texto + i) == '"' && last == ',')
        {
            i++;
            while (*(line->texto + i) != '"' && *(line->texto + i) != '\0')
                i++;
            last = 'e';
            i++;            
        }
        else if (*(line->texto + i) == ',' && last == 'e')
        {
            i++;
            last = ',';
        }
        else
        {
            free(str);
            line->error = 4;
            return;
        }
        while (*(line->texto + i) == ' ')
            i++;
    }
    if (*(line->texto + i) == '\0')
    {
        line->error = 2;
        free(str);
        return;
    }
    free(str);
    return;
}

void liberarTodo(struct linea lineas[], int numLineas, struct var vars[])
{
    int i;
    for (i = 0; i < numLineas; i++)
        free(lineas[i].texto);
    free(lineas);

    for (i = 0; i < numVariables; i++)
        free(vars[i].nombre);
    free(vars);
    return;
}

void evaluarLinea(struct linea *line)
{
    int i = 0, j = 0;
    char carac, *str;

    while (*(line->texto + i) == ' ') //avanza e ignora todos los posibles espacios en blanco al inicio de una linea
        i++;
    if (isalnum(*(line->texto + i)))
    {
        while (isalnum(*(line->texto + i + j)))
            j++;
        str = malloc(j + 1);
        strncpy(str, (line->texto + i), j);
        str[j] = '\0';
        str[0] = toupper(str[0]);
        if(!strcmp(str, "Leer") || !strcmp(str, "Escribir"))
        {
            if (str[0] == 'L')
            {
                line->tipo = LECTURA;
                lectura(line);
                free(str);
                return;
            }
            else
            {
                line->tipo = ESCRITURA;
                escritura(line);
                free(str);
                return;
            }
        }
        
        while (isalnum(*(line->texto + i))) //avanza todos los caracteres alfanumericos que haya, representaria una palabra entera sin espacios
            i++;
        while (*(line->texto + i) == ' ') //igual que al inicio ignora los espacios
            i++;

        if (isalnum(*(line->texto + i)))
        {
            line->tipo = DECLARACION;
            declaracion(line);
            free(str);
            return;
        }
        else if (*(line->texto + i) == '=')
        {
            line->tipo = ASIGNACION;
            asignacion(line);
            free(str);
            return;
        }
        else if (*(line->texto + i) == '(')
        {
            if (str[0] == 'L')
            {
                line->tipo = LECTURA;
                lectura(line);
                free(str);
                return;
            }
            else if (str[0] == 'E')
            {
                line->tipo = ESCRITURA;
                escritura(line);
                free(str);
                return;
            }
        }
    }
    line->tipo = NA;
    return; //si el primer caracter de la linea no es alfanumerico se retornara inmediatamente y queda como indeterminado
}

void imprimirErrores(struct linea lineas[], int numLineas)
{
    int i;
    FILE *archivo;
    archivo = fopen("corregido.txt", "w");
    for (i = 0; i < numLineas; i++)
    {
        if(lineas[i].error || !lineas[i].tipo)
        {
            fprintf(archivo, "Error en la linea: %i. ", i + 1);
            switch (lineas[i].tipo)
            {
            case 0:
                fprintf(archivo, "No se pudo identificar el tipo de instruccion, revise la sintaxis\n");
                break;
            case DECLARACION:
                switch (lineas[i].error)
                {
                case 1:
                    fprintf(archivo, "Escribio el tipo de dato incorrectamente, recuerde que debe empezar por minuscula y revise cuales son validos\n");
                    break;
                case 2:
                    fprintf(archivo, "Escribio incorrectamente el identificador, debe empezar por minuscula y no tener caracteres especiales\n");
                    break;
                case 3:
                    fprintf(archivo, "La variable que intenta declarar ya esta declarada\n");
                    break;
                }
                break;
            case ASIGNACION:
                switch (lineas[i].error)
                {
                case 1:
                    fprintf(archivo, "Una variable de las que intenta usar no ha sido declarada anteriormente\n");
                    break;
                case 2:
                    fprintf(archivo, "No se encuentra el signo = en el lugar requerido por la sintaxis\n");
                    break;
                case 3:
                    fprintf(archivo, "Existe un elemento incorrecto en la expresion de asignacion\n");
                    break;
                case 4:
                    fprintf(archivo, "Tiene un problema con la sintaxis de la expresion\n");
                    break;
                }
                break;
            case LECTURA:
                switch (lineas[i].error)
                {
                case 1:
                    fprintf(archivo, "La palabra Leer no esta escrita correctamente, recuerde la mayuscula\n");
                    break;
                case 2:
                    fprintf(archivo, "Los parentesis no se encuentran correctos en la expresion recuerde la estructura \"Leer(var)\"\n");
                    break;
                case 3:
                    fprintf(archivo, "Una variable de las que intenta usar no ha sido declarada anteriormente\n");
                    break;
                }
                break;
            case ESCRITURA:
                switch (lineas[i].error)
                {
                case 1:
                    fprintf(archivo, "La palabra Escribir no esta escrita correctamente, recuerde la mayuscula\n");
                    break;
                case 2:
                    fprintf(archivo, "Los parentesis no se encuentran correctos en la expresion recuerde la estructura \"escribir(expresion)\"\"\n");
                    break;
                case 3:
                    fprintf(archivo, "Una variable de las que intenta usar no ha sido declarada anteriormente\n");
                    break;
                case 4:
                    fprintf(archivo, "Existe un problema en la sintaxis de la expresion dentro de los parentesis\n");
                    break;
                }
                break;
            }
        }
    }
    fclose(archivo);
}

int main()
{
    int i, numLineas;
    struct linea *lineas;
    numLineas = contarLineas();
    lineas = malloc(sizeof(struct linea) * numLineas);
    extraerLineas(lineas, numLineas);
    for(i = 0; i < numLineas; i++)
        evaluarLinea(&lineas[i]);
    imprimirErrores(lineas, numLineas);
    liberarTodo(lineas, numLineas, vars);
    getch();
    return 0;
}