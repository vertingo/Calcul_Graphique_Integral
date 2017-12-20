///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Rivest Shamir Adleman ou RSA est un algorithme asym�trique de cryptographie � cl� publique
//
///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Un peu de th�orie :
//----------------------
//
// 1 - Cr�ation des cl�s :
//   - On choisit p et q deux nombres premiers distincts
//   - On note n leur produit, appel� module de chiffrement : n = p.q
//   - On calcule phi = (p-1)(q-1). C'est l'indicatrice d'Euler de n.
//   - On choisit e un entier premier avec phi, appel� exposant de chiffrement.
//     Dans la pratique, on peut prendre pour e un petit premier (par ex. 65537).
//   - Comme e est premier avec phi, il est inversible modulo phi
//     (i.e. il existe un entier d tel que e.d % phi = 1)
//     d est l'exposant de d�chiffrement.
//     Il est important que d soit un tr�s grand nombre.
// --> Le couple (n,e) est appel� clef publique et le couple (n,d) est appel� clef priv�e.
//
// 2 - Chiffrement du message :
//   - Soit M un entier inf�rieur � n repr�sentant un message.
//     Le message chiffr� C = ( M ^ e ) % n
//     Le message d�chiffr� M' = ( C ^ d ) % n
//     avec evidemment M = M'
//
// (merci wiki : http://fr.wikipedia.org/wiki/Rivest_Shamir_Adleman)
//
// Remarque non d�nu�e d'inter�t :
//----------------------------------
// Toutes les fonctions qui manipulent des big_int s'impl�mentent facilement avec
// la librairie (gratuite & magnifique) gmp (GNU Multiple Precision Arithmetic)
// (http://gmplib.org) ce qui permet de donner � ce cas d'�cole un niveau de s�curti�
// tr�s acceptable.
//
// Hadrien Flammang - dec 2008
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <cstdlib> // juste pour rand
#include "mini_rsa.h"

using namespace mini_rsa ;

// renvoie un bit al�atoire
static int random_bit ()
    {
    return( rand()&1 ) ;
    }

// retourne le PGCD de a et b (qui doivent �tre positifs)
static big_int pgcd ( big_int a , big_int b ) // thank's to Euclide
    {
    // return( a ? pgcd( a,b%a ) : b ) ; // version r�cursive
    big_int c = a % b ;
    while ( c != 0 )
        {
        a = b ;
        b = c ;
        c = a % b ;
        }
    return( b ) ;
    }

// retourne la partie enti�re de la racine carr�e de x (qui doit �tre positif)
static big_int sqrt ( big_int x ) // thank's to sir Isaac
    {
    big_int r = x ;
    for (;;)
        {
        big_int xx = (x/r + r) / 2 ;
        if ((r == xx) || (r == xx-1)) return( r ) ;
        r = xx ;      // ^---------^ arrive quand x est un carr� -1
        }
    }

// retourne true ssi x est impair
static bool is_odd ( big_int x )
    {
    return((x & 1) == 1 ) ;
    }

// retourne la partie enti�re du log en base 2 de x
// i.e. rang du bit de poids fort (log( 1 ) == 0, log( 2 ) == 1, log( 256 ) == 8, etc)
static int log ( big_int x )
    {
    int res = 0 ;
    for (; x > 0 ; x /= 2 ) res++ ;
    return( res ) ;
    }

// calcule ca et cb tels que a*ca + b*cb = pgcd( a,b )
// attention : ca et/ou cb peuvent �tre n�gatifs
static void euclide_etendu ( big_int a , big_int b , big_int * ca , big_int * cb )
    {
    big_int c,d,p,q,r,s,r2,s2 ;
    p = 1 ;
    q = 0 ;
    r = 0 ;
    s = 1 ;
    while ( b != 0 )
        {
        c = a % b ;
        d = a / b ;
        a = b ;
        b = c ;
        r2 = p - d*r ;
        s2 = q - d*s ;
        p = r ;
        q = s ;
        r = r2 ;
        s = s2 ;
        }
    if (ca) *ca = p ;
    if (cb) *cb = q ;
    }

// retourne true ssi x est premier
static bool is_prime ( big_int x )
    {
    // �limine les cas triviaux + divisibles par 2 ou 3
    if ( x < 2 ) return( false ) ;
    if ((x == 2) || (x == 3)) return( true ) ;
    if ((x % 2) == 0) return( false ) ;
    if ((x % 3) == 0) return( false ) ;
    // ne teste plus la divisibilit� par 2 ou 3
    big_int sx = sqrt( x ) ;
    big_int d  = 5 ;
    for ( int dd = 2 ; d <= sx ; d += dd , dd = 6-dd )
        if ((x % d) == 0) // ruse : d n'est multiple ni de 2 ni de 3
            return( false ) ;
    return( true ) ;
    }

// renvoie un nombre al�atoire dans { 2^nb_bit ... 2^(nb_bit+1)-1 }
static big_int big_random ( int nb_bit )
    {
    big_int res = 1 ; // assure que le bit de poids fort est � 1
    for ( --nb_bit ; nb_bit > 0 ; --nb_bit )
        res = res*2 + random_bit() ;
    return( res ) ;
    }

// renvoie un nb premier dans { 2^nb_bit ... 2^(nb_bit+1)-1 }
static big_int find_big_prime ( int nb_bit )
    {
    big_int res = big_random( nb_bit ) ;
    if (!is_odd( res )) ++res ;
    while ( !is_prime( res ))
        res += 2 ;  // bug potentiel : s'il n'y a aucun premier entre le res initial et
    return( res ) ; // 2^(nb_bit+1)-1 on va d�passer 2^(nb_bit+1)-1 ce qui peut �tre f�cheux.
    }

// retourne y tel que x et y soient premiers entre eux
static big_int find_prime_with ( int nb_bit , big_int x )
    {
    big_int res = big_random( nb_bit ) ;
    while ( pgcd( res,x ) != 1 )
        ++res ;
    return( res ) ;
    }

// retourne y tel que ( x*y ) % m == 1
static big_int inverse ( big_int x , big_int m )
    {
    big_int res ;
    euclide_etendu( x,m,&res,NULL ) ;
    while ( res < 1 ) res += m ;
    return( res ) ;
    }

// calcule x^y % m
static big_int modulus_exp ( big_int x , big_int y , big_int m )
    {
    if (y <= 0) return( 1 ) ;
    big_int res = 1 ;
    for ( x %= m ; y > 0 ; y /= 2 , x = (x*x)%m )
        if (is_odd( y ))
            res = (res*x)%m ;
    return( res ) ;
    }

// calcule un jeu de clefs RSA
static void compute_keys ( int nb_bit , big_int * modulus , big_int * private_exponant , big_int * public_exponant , bool symetric = false )
    {
    do  {
        big_int p,q ;
        do  {
            p = find_big_prime( (nb_bit+1)/2 ) ;

            do  q = find_big_prime( nb_bit/2 ) ;
                while ( p == q ) ; // p et q doivent �tre distincts

            *modulus = p*q ;
            }
            while ( log( *modulus ) != nb_bit ) ; // le modulo doit �tre assez grand mais pas trop

         big_int phi = (p-1)*(q-1) ;

         if (symetric) // on veut que la clef publique soit ~ aussi grande que la clef priv�e
            *public_exponant = find_prime_with( nb_bit,phi ) ;
         else
            *public_exponant = 65537 ; // on trouve souvent 65537 dans la litterature

        *private_exponant = inverse( *public_exponant,phi ) ;
        }
        // On recommence jusqu'� ce que l'exposant de d�chiffrement (private_exponant) soit assez grand
        // et different de l'exposant de chiffrement
        while ((log( *private_exponant ) < nb_bit-1) || (*public_exponant == *private_exponant)) ;
    }

//-----------------------------------------------------------------------------

namespace mini_rsa
{

void compute_keys ( int nb_bit , big_int * pub , big_int * pri , big_int * mod )
    {
    ::compute_keys( nb_bit,mod,pri,pub ) ;
    }

big_int encrypt ( big_int exp , big_int mod , big_int message )
    {
    return( modulus_exp( message,exp,mod )) ;
    }

big_int random ( int nb_bit )
    {
    return( big_random( nb_bit )) ;
    }

}
