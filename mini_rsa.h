#ifndef __mini_rsa_h__
#define __mini_rsa_h__


namespace mini_rsa
{

// type des entiers utilis�s comme clef
typedef long long big_int ;  // doit �tre un type entier sign�

// partie enti�re du log en base 2 de la plus grande valeur possible d'un 'big_int' (ici log( 2^63-1 ) == 62)
const int log_big_int = 62 ; // attention : cette valeur doit �tre mise � jour si bit_int est chang�.

// nb max de bits utilisables
const int max_nb_bit = log_big_int/2 ; // comme on a des multiplications, on ne peut utiliser que la moiti� des bits

// renvoie un entier al�atoire sur nb_bit bits (le bit de poids fort �tant toujours � 1)
// cette fonction n'est l� que pour les tests.
big_int random ( int nb_bit ) ;

// g�n�re une bi-clef (= paire clef publique + clef priv�e) sur nb_bit
// la clef priv�e = paire ( pri,mod ) et la clef publique = paire ( pub,mod )
void compute_keys ( int nb_bit , big_int * pub , big_int * pri , big_int * mod ) ;

// chiffrement (= d�chiffrement avec la clef r�ciproque)
// 'message' contient le message � chiffrer/dechiffrer sous forme d'un
// entier qui doit �tre inferieur � 'mod'.
// le param�tre 'exp' doit recevoir le 'pub' ou le 'pri' sorti de 'compute_keys'.
// le param�tre 'mod' doit recevoir le 'mod' sorti de 'compute_keys'.
// le param�tre 'message' contient le message � chiffrer.
//    (Attention : C'est l'appelant qui doit convertir son buffer de donn�es en 'big_int'.)
// le retour est le message chiffr�.
big_int encrypt ( big_int exp , big_int mod , big_int message ) ;

}

#endif // __mini_rsa_h__
