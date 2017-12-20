#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "interpreteur.h"
#include "interpreteur_c.h"

/* Supprimez la ligne suivante pour entrer en mode debogage */
#define EVALUER_NO_DEBUG

#ifdef  EVALUER_NO_DEBUG
#	define EvaluerDebug Ignorer
#else
#	include <stdio.h>
#	define EvaluerDebug printf
#endif

void InterpreteurInit()
{
	TabOperations[P_O].priorite          = PRIOR_P_O;
	TabOperations[P_O].associativite     = GAUCHE;
	TabOperations[P_O].nbArgs            = 0;
	TabOperations[P_O].p_fonction        = NULL;

	TabOperations[P_F].priorite          = PRIOR_P_F;
	TabOperations[P_F].associativite     = GAUCHE;
	TabOperations[P_F].nbArgs            = 0;
	TabOperations[P_F].p_fonction        = NULL;

	TabOperations[END].priorite          = PRIOR_END;
	TabOperations[END].associativite     = GAUCHE;
	TabOperations[END].nbArgs            = 0;
	TabOperations[END].p_fonction        = NULL;

	TabOperations[PLUS].priorite         = PRIOR_PLUS_MOINS;
	TabOperations[PLUS].associativite    = GAUCHE;
	TabOperations[PLUS].nbArgs           = 2;
	TabOperations[PLUS].p_fonction       = Somme;

	TabOperations[MOINS].priorite        = PRIOR_PLUS_MOINS;
	TabOperations[MOINS].associativite   = GAUCHE;
	TabOperations[MOINS].nbArgs          = 2;
	TabOperations[MOINS].p_fonction      = Diff;

	TabOperations[FOIS].priorite         = PRIOR_FOIS_DIV;
	TabOperations[FOIS].associativite    = GAUCHE;
	TabOperations[FOIS].nbArgs           = 2;
	TabOperations[FOIS].p_fonction       = Prod;

	TabOperations[DIV].priorite          = PRIOR_FOIS_DIV;
	TabOperations[DIV].associativite     = GAUCHE;
	TabOperations[DIV].nbArgs            = 2;
	TabOperations[DIV].p_fonction        = Div;

	TabOperations[POW].priorite          = PRIOR_POW;
	TabOperations[POW].associativite     = GAUCHE;
	TabOperations[POW].nbArgs            = 2;
	TabOperations[POW].p_fonction        = Pow;

	TabOperations[OPP].priorite          = PRIOR_FONCTION;
	TabOperations[OPP].associativite     = DROITE;
	TabOperations[OPP].nbArgs            = 1;
	TabOperations[OPP].p_fonction        = Opp;

	TabOperations[SIN].priorite          = PRIOR_FONCTION;
	TabOperations[SIN].associativite     = DROITE;
	TabOperations[SIN].nbArgs            = 1;
	TabOperations[SIN].p_fonction        = Sin;

	TabOperations[COS].priorite          = PRIOR_FONCTION;
	TabOperations[COS].associativite     = DROITE;
	TabOperations[COS].nbArgs            = 1;
	TabOperations[COS].p_fonction        = Cos;

	TabOperations[TAN].priorite          = PRIOR_FONCTION;
	TabOperations[TAN].associativite     = DROITE;
	TabOperations[TAN].nbArgs            = 1;
	TabOperations[TAN].p_fonction        = Tan;

	TabOperations[ABS].priorite          = PRIOR_FONCTION;
	TabOperations[ABS].associativite     = DROITE;
	TabOperations[ABS].nbArgs            = 1;
	TabOperations[ABS].p_fonction        = Abs;

	TabOperations[RACINE].priorite       = PRIOR_FONCTION;
	TabOperations[RACINE].associativite  = DROITE;
	TabOperations[RACINE].nbArgs         = 1;
	TabOperations[RACINE].p_fonction     = Racine;

	TabOperations[LOG].priorite          = PRIOR_FONCTION;
	TabOperations[LOG].associativite     = DROITE;
	TabOperations[LOG].nbArgs            = 1;
	TabOperations[LOG].p_fonction        = Log;

	TabOperations[LOG10].priorite        = PRIOR_FONCTION;
	TabOperations[LOG10].associativite   = DROITE;
	TabOperations[LOG10].nbArgs          = 1;
	TabOperations[LOG10].p_fonction      = Log10;

	TabOperations[EXP].priorite          = PRIOR_FONCTION;
	TabOperations[EXP].associativite     = DROITE;
	TabOperations[EXP].nbArgs            = 1;
	TabOperations[EXP].p_fonction        = Exp;
}

void InterpreteurReset()
{
	Operations.NbElements = 0;
	Operations.t[0] = INVALIDF;
	Operandes.NbElements = 0;
}

int Calculer(char * lpszString, LPPARAM lpParams, size_t nbParams, double * lpResult)
{
	int ret = EVALUER_SUCCESS;

	if ((lpszString != NULL) && (lpResult != NULL))
	{
		char * lpMot = NULL;
		int c, sep;
		/* c   : caractere courant */
		/* sep : caractere precedent */
		size_t i = 0;

		ret = EnleverEspaces(lpszString);

		lpMot = lpszString;
		sep = c = lpMot[i];

		while (ret == EVALUER_SUCCESS)
		{
			if (IsSeparator(c))
			{
			    /*
			    1. On empile l'eventuel mot
			    2. On empile le separateur
			    3. On deplace le pointeur pMot vers le mot suivant
			    4. On remet i a zero
			    */

			    /* Si c est l'operateur -, trouver sa signification */
				if (c == '-' && i == 0 && sep != ')')
					c = 'o';

				if (i > 0)
					ret = EmpilerMot(lpMot, i, lpParams, nbParams);

				if (ret == EVALUER_SUCCESS)
				{
					ret = EmpilerSep(c);

					if (c == '\0')
						break;
					else
						if (ret == EVALUER_SUCCESS)
						{
							lpMot += (i + 1);
							i = 0;
						}
				}
			}
			else
				i++;

            sep = c;
			c = lpMot[i];
		}

		if (ret == EVALUER_SUCCESS)
		{
			if (Operandes.NbElements == 1)
                if (Operations.NbElements == 0)
                    *lpResult = Operandes.t[0];
                else
					if (Operations.t[Operations.NbElements - 1] == P_O)
						ret = EVALUER_P_F_MANQUANTE;
					else
						ret = EVALUER_FEW_ARGS;
			else
				ret = EVALUER_TOO_MUCH_ARGS;
		}
	}
	else
		ret = EVALUER_NULL_ARG;

	return ret;
}

int EnleverEspaces(char * lpszString)
{
	int ret = EVALUER_SUCCESS;
	char * lpBuffer;

	lpBuffer = malloc(strlen(lpszString) + 1);
	if (lpBuffer != NULL)
	{
		int i = 0, j = 0;
		char c;

		for(i = 0; (c = lpszString[i]) != '\0'; i++)
			if (!isspace(c))
				lpBuffer[j++] = c;
		lpBuffer[j] = '\0';

		strcpy(lpszString, lpBuffer);
		free(lpBuffer);
	}
	else
		ret = EVALUER_MEMOIRE_INSUFFISANTE;

	return ret;
}

BOOL IsSeparator(int c)
{
	BOOL ret;

	switch(c)
	{
	case '+': case '-': case '*': case '/': case '^': case '(': case ')': case '\0':
		ret = TRUE;
		break;

	default:
		ret = FALSE;
	}

	return ret;
}

int EmpilerMot(char * lpMot, size_t cchMot, LPPARAM lpParams, size_t nbParams)
{
	int ret = EVALUER_SUCCESS;
	char * lpszMot = NULL;

	lpszMot = malloc(cchMot + 1);
	if (lpszMot != NULL)
	{
		double x;
		char * p_stop = NULL;

		strncpy(lpszMot, lpMot, cchMot);
		lpszMot[cchMot] = '\0';

		x = strtod(lpszMot, &p_stop);

		/* Si la conversion s'est bien deroulee */
		if (p_stop - lpszMot == (ptrdiff_t)cchMot)
		{
		    /* c'est un nombre (une operande) */
			EvaluerDebug("EmpilerOperande %g\n", x);
			ret = EmpilerOperande(x);
		}
		else
		{
		    /* c'est peut etre le nom d'une fonction ou un parametre ... */
			ret = EmpilerChaine(lpszMot, lpParams, nbParams);
		}

		free(lpszMot);
	}
	else
		ret = EVALUER_MEMOIRE_INSUFFISANTE;

	return ret;
}

int EmpilerOperande(double x)
{
	int ret = EVALUER_SUCCESS;

	if (Operandes.NbElements < MAX_OPERANDES)
	{
		Operandes.t[Operandes.NbElements] = x;
		Operandes.NbElements++;
	}
	else
		ret = EVALUER_PILE_PLEINE;

	return ret;
}

int EmpilerChaine(char * lpszMot, LPPARAM lpParams, size_t nbParams)
{
	int fonction = INVALIDF, ret = EVALUER_SUCCESS;

	if (!strcmp(lpszMot, "sin"))
		fonction = SIN;

	if (!strcmp(lpszMot, "cos"))
		fonction = COS;

	if (!strcmp(lpszMot, "tan"))
		fonction = TAN;

	if (!strcmp(lpszMot, "abs"))
		fonction = ABS;

	if (!strcmp(lpszMot, "sqrt"))
		fonction = RACINE;

	if (!strcmp(lpszMot, "ln"))
		fonction = LOG;

	if (!strcmp(lpszMot, "log"))
		fonction = LOG10;

	if (!strcmp(lpszMot, "exp"))
		fonction = EXP;

	if (fonction != INVALIDF)
	{
		EvaluerDebug("EmpilerFonction %s\n", lpszMot);
		ret = EmpilerFonction(fonction);
	}
	else
	{
		EvaluerDebug("EmpilerParam %s = ", lpszMot);
		ret = EmpilerParam(lpszMot, lpParams, nbParams);
	}

	return ret;
}

int EmpilerSep(int c)
{
	int ret = EVALUER_SUCCESS;

	if (Operations.NbElements < MAX_OPERATIONS)
	{
		int operation;

		switch(c)
		{
		case '+':
			operation = PLUS;
			break;

		case '-':
			operation = MOINS;
			break;

		case '*':
			operation = FOIS;
			break;

		case '/':
			operation = DIV;
			break;

		case '^':
			operation = POW;
			break;

		case '(':
			operation = P_O;
			break;

		case ')':
			operation = P_F;
			break;

		case '\0':
			operation = END;
			break;

		case 'o':
			operation = OPP;
			break;

		default:
			ret = EVALUER_OPERATION_INCONNUE;
		}

		if (ret == EVALUER_SUCCESS)
		{
			EvaluerDebug("EmpilerFonction %c\n", c == 'o' ? '-' : (c == '\0' ? '$' : c));
			ret = EmpilerFonction(operation);
		}
	}
	else
		ret = EVALUER_PILE_PLEINE;

	return ret;
}

int EmpilerFonction(int f)
{
	int p_o = 0, ret = EVALUER_SUCCESS;

	if (f != P_O)
	{
		int operation_courante;

		operation_courante = Operations.t[Operations.NbElements - 1];

		while ((Operations.NbElements > 0) && Prioritaire(operation_courante, f))
		{
			EvaluerDebug("IRQ!\n");
			p_o = (operation_courante == P_O);
			ret = Evaluer();

			if ((ret != EVALUER_SUCCESS) || ((f == P_F) && (p_o)))
				break;

			if (Operations.NbElements > 0)
				operation_courante = Operations.t[Operations.NbElements - 1];
		}
	}

	if (ret == EVALUER_SUCCESS)
	{
		if ((f == P_F) || (f == END))
		{
			if ((f == P_F) && (!p_o))
				ret = EVALUER_P_O_MANQUANTE;
		}
		else
		{
			if (Operations.NbElements < MAX_OPERATIONS)
			{
				Operations.t[Operations.NbElements] = f;
				Operations.NbElements++;
			}
			else
				ret = EVALUER_PILE_PLEINE;
		}
	}

	return ret;
}

BOOL Prioritaire(int operation, int second_operation)
{
	int prior_operation, prior_second, associativite;

	prior_operation = TabOperations[operation].priorite;
	prior_second = TabOperations[second_operation].priorite;
	associativite = TabOperations[operation].associativite;

	return ((prior_operation > prior_second) || ((prior_operation == prior_second) && (associativite == GAUCHE)));
}

int EmpilerParam(char * lpszParam, LPPARAM lpParams, size_t nbParams)
{
	int ret = EVALUER_SUCCESS;

	if (lpParams != NULL)
	{
		size_t i;
		BOOL found = FALSE;
		double x;

        /* On recherche lpszParam dans lpParams */
		for(i = 0; i < nbParams; i++)
			if (!strcmp(lpParams[i].lpszName, lpszParam))
			{
				found = TRUE;
				x = lpParams[i].Value;
				break;
			}

		if (found)
		{
		    /* on empile la valeur du parametre */
			EvaluerDebug("EmpilerOperande %g\n", x);
			ret = EmpilerOperande(x);
		}
		else
		{
			EvaluerDebug("Parametre Indefini !\n");
			ret = EVALUER_PARAM_INDEFINI;
		}
	}
	else
		ret = EVALUER_PARAM_INDEFINI;

	return ret;
}

int Evaluer()
{
	int ret = EVALUER_SUCCESS;

	if (Operations.NbElements > 0)
	{
		int operation;
		size_t nbArgs, nbOperandes;
		void * p_fonction;

		operation = Operations.t[Operations.NbElements - 1];
		nbArgs = TabOperations[operation].nbArgs;
		p_fonction = TabOperations[operation].p_fonction;

		nbOperandes = Operandes.NbElements;

		if (nbOperandes >= nbArgs)
		{
			if (p_fonction != NULL)
			{
				switch(nbArgs)
				{
				case 1:
					Operandes.t[nbOperandes - 1] = ((F1VAR)p_fonction)(Operandes.t[nbOperandes - 1]);
					break;

				case 2:
					Operandes.t[nbOperandes - 2] = ((F2VAR)p_fonction)(Operandes.t[nbOperandes - 2], Operandes.t[nbOperandes - 1]);
					Operandes.NbElements--;
					break;

				default:
					ret = EVALUER_UNSUPPORTED_OPERATION;
				}
			}

			Operations.NbElements--;
		}
		else
			ret = EVALUER_FEW_ARGS;
	}
	else
		ret = EVALUER_OPERATION_MANQUANTE;

	return ret;
}

double Somme(double a, double b)
{
	double y;
	y = a + b;
	EvaluerDebug("%g + %g = %g\n", a, b, y);
	return y;
}

double Diff(double a, double b)
{
	double y;
	y = a - b;
	EvaluerDebug("%g - %g = %g\n", a, b, y);
	return y;
}

double Prod(double a, double b)
{
	double y;
	y = a * b;
	EvaluerDebug("%g * %g = %g\n", a, b, y);
	return y;
}

double Div(double a, double b)
{
	double y;
	y = a / b;
	EvaluerDebug("%g / %g = %g\n", a, b, y);
	return y;
}

double Pow(double a, double b)
{
	double y;
	y = pow(a, b);
	EvaluerDebug("%g ^ %g = %g\n", a, b, y);
	return y;
}

double Opp(double x)
{
	double y;
	y = -x;
	EvaluerDebug("-%g = %g\n", x, y);
	return y;
}

double Sin(double x)
{
	double y;
	y = sin(x);
	EvaluerDebug("sin(%g) = %g\n", x, y);
	return y;
}

double Cos(double x)
{
	double y;
	y = cos(x);
	EvaluerDebug("cos(%g) = %g\n", x, y);
	return y;
}

double Tan(double x)
{
	double y;
	y = tan(x);
	EvaluerDebug("tan(%g) = %g\n", x, y);
	return y;
}

double Abs(double x)
{
	double y;
	y = fabs(x);
	EvaluerDebug("abs(%g) = %g\n", x, y);
	return y;
}

double Racine(double x)
{
	double y;
	y = sqrt(x);
	EvaluerDebug("sqrt(%g) = %g\n", x, y);
	return y;
}

double Log(double x)
{
	double y;
	y = log(x);
	EvaluerDebug("ln(%g) = %g\n", x, y);
	return y;
}

double Log10(double x)
{
	double y;
	y = log10(x);
	EvaluerDebug("log(%g) = %g\n", x, y);
	return y;
}

double Exp(double x)
{
	double y;
	y = exp(x);
	EvaluerDebug("exp(%g) = %g\n", x, y);
	return y;
}

int Ignorer(char const * s, ...)
{
    return (int)strlen(s);
}