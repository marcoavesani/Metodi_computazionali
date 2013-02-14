//
// Programma: automa_final_final.cxx
//
// Descrizione: Studio del traffico su una strada a due corsie con possibilità di sorpasso e rientro in corsia e con svincolo su corsia destra.
//
// 
//
// Data:13/02/2013
//

#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <libgen.h>
#include <stdlib.h>
#include <cmath>
#include <vector>
#include "fstream"
#include "time.h"
#include "algorithm"
#include "boost/random.hpp"
#include "root/TGraph.h"
#include "root/TCanvas.h"
#include "root/TApplication.h"
#include "root/TFile.h"
#include "root/TH2F.h"
#include "root/TGraphTime.h"
#include "root/TMarker.h"
#include "root/Rtypes.h"

using namespace std;

struct car { // Definizione della struttura macchina che conterrà per comodità praticamente tutte le informazioni rilevanti
    int x;
    int v;
    int corsia;
    int numero;
    bool uscita; //L'uscita è una proprietà della macchina, scelta in fase di creazione, in quanto si sa a priori se bisogna uscire allo svincolo oppure no
    bool flusso;
  
};

bool ordinamento(const car& a, const car& b)  //Funzione x il sort in modo da ordinare il vettore in base alla posizione
{
 if( a.x==b.x && a.corsia==b.corsia) cout<<"ERRORE CORSIA POSIZIONE"<<endl; // Debug
  return a.x>b.x;
}


void usage(const char * pname)  // 
{
    cerr << "Usage: " << pname << " [options] " << endl;
    cerr << "\nStudio del traffico su una strada rettilinea" << endl;
    cerr << "\nOptions:\n";
    cerr << " -p prob  : probabilita' di rallentamento (default = 0.5)\n";
    cerr << " -q prob  : probabilita' di ingresso (default = 0.2)\n";
    cerr << " -l lung  : Lunghezza della strada (default = 100 unita')\n";
    cerr << " -v max   : Velocita' massima automobili (default = 5 unita')\n";
    cerr << " -s prob  : probabilità di sorpasso (default = ) \n";
    cerr << " -r prob  : probabilità di rientro (default = ) \n";
    cerr << " -u prob  : probabilità di uscita (default = ) \n";
    cerr << " -o nomefile  : nome del file in cui verranno salvati i dati \n";
    cerr << " -g       : abilita l'output grafico \n";
    cerr << " -t num   : sceglie il seed ( 0 usa il tempo macchina ) (default=10) \n";
    cerr << " -f num   : sceglie il punto in cui calcolare il flusso \n";
    cerr << " -a       : include l'animazione \n";
  
}
/*
void print_debug(const vector<car> & z) // Funzione di debug ( è stata MOLTO utile)
{
      cout << "Automobili: " << z.size() << endl;
    for (int i=0; i<z.size(); i++) {
      cout << "i: " << i << " x: " << z[i].x<< " v: " << z[i].v <<"numero:  "<< z[i].numero<< endl;
    }
}
*/
void print_strada( vector<car> & z, int street_length)  //Funzione per la visualizzazione della strada.. Prende in ingresso una refernza a un vettore di macchine e la lunghezza della strada.
{   //Inizializzazione
    int * carreggiata1 = new int[street_length-1];  // creo un'array dinamico di int e setto tutto a -1.. In questo modo posso visualizzare le macchine con il loro numero... Più facile da seguire
    int * carreggiata2 = new int[street_length-1];
    //Svuotamento
    for (int i=0; i<street_length; i++) {
        carreggiata1[i] = -1;  // Uno x corsia
	carreggiata2[i]= -1;
      
    }
   
   //Riempimento
   for(  vector<car>::iterator iter = z.begin(); // Qua faccio un po di magie con i vettori. inizializzo degli iteratori per poter gestire gli elementi del vettore
         iter != z.end();
         iter++) 
       {

        int position = iter->x;  // -> è usato al posto di . con le strutture dinamiche. Sto accedendo al valore x dell'elemento dato dall'iteratore.
	int numero_macchina=iter->numero;
	
	if (iter->corsia == 0)			// Controllo il valore di corsia per ogni elemento. Se appartiene alla prima metto il carattere * sul primo array altrimenti sul secondo ( non uso else nel caso volessi aggiungere altre carreggiate)
        carreggiata1[position] = iter->numero;
	
	if (iter->corsia == 1)
	carreggiata2[position] = iter->numero;
	}

//Visualizzazione 
      for (int i=0; i<street_length;i++)	
      { 
	if (carreggiata2[i]<0){ 		// Segno un'elemento vuoto della strada con un ., la posizione della macchina è rappresentata dal suo numero.
	cout<<".";}
	else
	cout <<carreggiata2[i];
      }
	cout<<endl;
    
     for (int i=0; i<street_length;i++)
     {
      if (carreggiata1[i]<0){
      cout<<".";}
      else
      cout <<carreggiata1[i];
    }
    cout<<endl;
 
    
    //  cout << carreggiata1<<endl;  // Stampo i due array... Futura implementazione output tramite TGraphTime
    
    cout << endl; // Due righe vuote per visualizzare meglio
    cout <<endl;
}

int main(int argc, char * argv[])
{
  // Inizializzazione Variabili//
    
    char nomefile[200];
    const char flusso_name[100]="flusso_densita.txt"; 
    int opt;
    int street_length = 100;
    int svincolo=75;
    int v_max = 2;
    int distanza_sorpasso=3;
    int contatore_macchine=0;
    int t_rimozione=0;
    int punto_controllo=60;
    int seed = 10;
    int velocita_media=0;
    int distanza_media=0;
    double p_stop = 0.5;
    double p_in = 0.2;
    double p_sorpasso = 0.5;
    double p_rientro = 0.3;
    double p_uscita =0.2;
    double densita=0.;
    double flusso=0.;
    bool graphics=false;
    bool rimozione=false; // Lo uso per vedere se una macchina è già stata rimossa dal grafico
    bool animazione=false;
     //Gestione I/O //
    
    ofstream out;
    
    ofstream flux_out;
    flux_out.open(flusso_name, ios::app);
    
    if ( flux_out.is_open()==false ) 
    {
      cout<<"ERRORE APERTURA FILE FLUSSO"<<endl;
    }
    
    // Gestione input programma da linea di comando //
    while((opt = getopt(argc, argv, "l:p:q:v:o:s:r:u:t:f:gah")) != -1) {

        switch (opt) {

        case 'p':
            p_stop = strtod(optarg, NULL);
            if (p_stop < 0 || p_stop > 1.0)  {
                cerr << "Probabilita' stop errata: '" << p_stop << "'\n";
                usage(basename(argv[0]));
                return 1;
            }
            break;

        case 'q':
            p_in = strtod(optarg, NULL);
            if (p_in < 0 || p_in > 1.0)  {
                cerr << "Probabilita' ingresso errata: '" << p_in << "'\n";
                usage(basename(argv[0]));
                return 1;
            }
            break;

        case 'v':
            v_max = atoi(optarg);
            if (v_max < 0 || v_max > 10) {
                cerr << "Velocita' automobili errata: \""
                     << v_max << "\"\n";
                usage(basename(argv[0]));
                return 1;
            }
            break;

        case 'l':
            street_length = atoi(optarg);
            if (street_length < 0 || street_length > 100) {
                cerr << "Dimensioni strada errate: \""
                     << street_length << "\"\n";
                usage(basename(argv[0]));
                return 1;
            }
            break;

        case 'o':
          out.open(optarg);
	  snprintf(nomefile,199,"%s",optarg);
            if ( out.is_open() == false){
	      cerr<< "Errore file di scrittura"<<endl;
	      usage(basename(argv[0]));
            }
            break;
	
	case 's':
            p_sorpasso = strtod(optarg, NULL);
            if (p_sorpasso < 0 || p_sorpasso > 1.0)  {
                cerr << "Probabilita' sorpasso errata: '" << p_sorpasso << "'\n";
                usage(basename(argv[0]));
                return 1;
            }
            break;
	
	case 'r':
            p_rientro = strtod(optarg, NULL);
            if (p_rientro < 0 || p_rientro > 1.0)  {
                cerr << "Probabilita' rientro errata: '" << p_rientro << "'\n";
                usage(basename(argv[0]));
                return 1;
            }
            break;
	
	case 'u':
            p_uscita = strtod(optarg, NULL);
            if (p_uscita < 0 || p_uscita > 1.0)  {
                cerr << "Probabilita' uscita errata: '" << p_uscita << "'\n";
                usage(basename(argv[0]));
                return 1;
            }
            break;
	
	case 'g':
            graphics = true;
            
            break;
	
	case 'f':
            punto_controllo = atoi(optarg);
            if (punto_controllo < 0 || punto_controllo > 100)  {
                cerr << "Punto di controllo errato: '" << punto_controllo << "'\n";
                usage(basename(argv[0]));
                return 1;
            }
            break;    
	
	case 't':
            seed= atoi(optarg);
           break;
	   
	case 'a':
            animazione=true;
           break;
	    
	case 'h':
        case '?':
        default:
            usage(basename(argv[0]));
            return 1;
        }
    }

     //Inizializzazione parte grafica
 
    TApplication* theApp = new TApplication("App", &argc, argv);
    TCanvas * c1 = new TCanvas("c1","Grafico"); // creo il canvas su cui plottare
    TCanvas * c2 = new TCanvas("c2","Grafico2"); // creo il canvas su cui plottare
    TCanvas * c3 = new TCanvas("c3","Grafico3"); // creo il canvas su cui plottare
    TCanvas * c4 = new TCanvas("c4","Grafico4"); // creo il canvas su cui plottare
    TCanvas * c5 = new TCanvas("c5","Grafico5"); // creo il canvas su cui plottare
    
    // Inizializzazione geneatore random //
    boost::uniform_real<> uni(0, 1); // Scelgo distribuzione uniforme
    boost::mt19937 rng;  // Scelgo generatore ( Mt è veloce)
    if (seed==0){
    rng.seed(time(NULL)); // Inizializzo il generatore col seed (se seed=0 uso il tempo macchina)
    }
    else
    rng.seed(seed);
    boost::variate_generator < boost::mt19937 &,
                               boost::uniform_real<> > uni_one(rng, uni);

   //Interazione con l'utente //

    cout << "Numero di iterazioni: ";
    int t_n;
    cin >> t_n;

    // crea un vettore di automobili
    vector <car> coda;
    const int tmax=t_n;
    Double_t tempo_array[tmax];
    Double_t vmedia_array[tmax], distanza_media_array[tmax];
    // Gestione oggetti da plottare //
    TH2F h2("h2","Tempo-Velocita",tmax,0,tmax,Int_t(v_max),0,Int_t(v_max));
    TH2F h3("h3","Tempo-Distanza Macchine",tmax,0,tmax,street_length,0,street_length);
    TGraphTime *g = new TGraphTime(tmax,0,-1,street_length,3);
    
    // Inseriamo un'auto  // Inizializzazione fuori dal ciclo.. Butto dento un'auto almento x farlo iniziare
    car c_one;
    c_one.x = 0;
    c_one.v = 0;
    c_one.corsia=0;
    c_one.flusso=false;
    c_one.numero=0;
    if (uni_one()<p_uscita){  // La probabilità d'uscita viene associata in fase di creazione. Suppongo un automobilista abbia già deciso quando imbocca la strada se uscire o meno
	c_one.uscita=true;}
     else{
	c_one.uscita=false;
     }
    coda.push_back(c_one);// Inserisco l'elemento nel vettore di macchine dal fondo
    contatore_macchine++; // Inizializzo un contatore per il numero delle macchine.. Necessario in quanto poi farò un sort
    out<< "#Tempo: "<<t_n<<"##############"<<endl;
    out<< "#Macchina numero: "<<c_one.numero<<" Posizione: "<<c_one.x<< " Velocità: " <<c_one.v<< " Corsia: "<<c_one.corsia<<endl;
    
    
    
    
    // Evolvo il sistema
   int t=0;
    while (t<=tmax) {
     
	out<< "#Tempo: "<<t<<"##############"<<endl; //Piu per debug che per altro
         cout << "-=-=-=-=> t: " << t << endl;

        // Inserisco un'automobile, se possibile
        if (uni_one() < p_in) {  // Check probabilità se maggiore
           
	  car last = coda.back(); // Ultima macchina presente nel vettore. La copio nella struttura temporanea last
	  car c_one;
            c_one.x = 0;
            c_one.v = 0;
            c_one.corsia=0;
	    c_one.corsia=0;
	    c_one.numero=(contatore_macchine);	// Identifico con dei numeri le macchine
	    if (uni_one()<p_uscita){          	// Scelgo ora se uscirà o meno allo svincolo
	      c_one.uscita=true;}
	    else{
	      c_one.uscita=false;
	     }
	    //Controllo se è possibile inserire//
	    if (coda.size() == 0) {  // Se la coda è vuota no problem
                coda.push_back(c_one);
		contatore_macchine++; 
	      } 
	    else {
	      if (last.x != 0) { // Se l'ultima macchina non si trova nella posizione 0 caccio tutto dentro
                  coda.push_back(c_one);
		  contatore_macchine++;
		  }
            }
        }

   // Da qua in poi c'è la parte importante//
      // Agisco sulle automobili
        for(int j=0; j<coda.size(); j++) {

            if (uni_one() < p_stop  ) {  //Tiro il dado se la quello che esce è più basso della probabilità di diminuire la velocità lo faccio
                if (coda[j].v > 0) {
                    coda[j].v--;
                }
            } else {
                // Aumento velocita' di una unita'
                if (coda[j].v < v_max) { 
                    coda[j].v++;
                }
            }


  //Riordino gli elementi. Molto meno dispendioso dal punto di vista computazionale per la gestione degli incidenti
  stable_sort(coda.begin(), coda.end(), ordinamento ); // Uso stable_sort invece di sort in quanto quest'ultimo non gestisce il caso dell'uguaglianza 
  // In generale gli dico di partire dall'inizio del vettore fino alla fine e di riordinarmelo in ordine decrescente per le posizioni

  int distance = coda[j-1].x - coda[j].x;
  
  // Gestione Sorpassi e rientri
  // Se stanno sulla stessa corsia vado avnti
  //Sorpasso: Se il dado mi da l'ok, se mi trovo sulla corsia di destra, se la posizione in cui voglio andare non è occupata e se la distanza dalla macchina davanti è inferiore a quella permessa per il sorpasso allora procedo
  
  if ((coda[j-1].corsia - coda[j].corsia)==0) {
       
    if ( j>0 && (uni_one()<= p_sorpasso) && (coda[j].corsia == 0) && (coda[j-1].x != coda[j].x)&& (coda[j+1].x != coda[j].x) && (distance <= distanza_sorpasso) ) 
	{
	  coda[j].corsia=1;
	}
  //Rientro:se il dado mi da l'ok e la posizione in cui voglio andare non è occupata procedo
    if ( (uni_one()<= p_rientro && coda[j].corsia == 1) && (coda[j-1].x != coda[j].x) && (coda[j+1].x != coda[j].x) ) 
	{
	  coda[j].corsia=0;
	}
	
    
  }
 //Svincolo//
 if ( (coda[j].uscita == true) && coda[j].x >=50 ){
   int dist_svincolo =svincolo-coda[j].x;
   
   if( (coda[j].corsia==1) &&  (coda[j-1].x != coda[j].x) && (coda[j+1].x != coda[j].x) ) {
      coda[j].corsia=0;
    }
 if(dist_svincolo>10){
 //coda[j].v= int((dist_svincolo-coda[j].v)/(dist_svincolo))*coda[j].v;
 coda[j].v=min( int(coda[j].v/2)+1,coda[j].v);
   
}
  else if (dist_svincolo<=5  && dist_svincolo>3){
    coda[j].v= min(2,coda[j].v);
  }
  else if(dist_svincolo<=3){ 
    coda[j].v=min(1,coda[j].v);}
  
  
  
  else if (dist_svincolo==0 ) {
    coda[j].v=0;
  }
}
 
 
  //Non voglio incidenti.
  //Controllo rispetto a tutte le macchine più avanti di me e sulla mia corsia che la mia velocità mi consenta di viaggiare senza sbattere adosso a quelli davanti.
 int dist_min=street_length; //Trovo la distanza tra la macchina e la sucessiva (la devo plottare)
 if(j>0){  
  for (int k=j-1;k>=0;k--)
    { if((coda[k].x - coda[j].x)<dist_min){ dist_min=(coda[k].x - coda[j].x);}
      if( (coda[j].v >= (coda[k].x - coda[j].x)) && ((coda[k].corsia - coda[j].corsia)==0) )
      {
	    coda[j].v = (coda[k].x - coda[j].x) - 1;
	
      }
    }
 }
 
  // Aggiorniamo le posizioni
    coda[j].x = coda[j].x + coda[j].v;
  
    if (coda[j].x>=punto_controllo && coda[j].flusso==false && rimozione==true) //Se la macchina ha passato il punto di controllo, se non è stata conteggiata e se almeno una macchina è già uscita dallo schermo la conteggio e la segno come conteggiata.
    {
	flusso=flusso+1;
	coda[j].flusso=true;
      
    }
    //Rimuovo le auto fuori dalla strada

    if (coda[j].x > street_length  ) {
                coda.erase( coda.begin() + j );
		if(rimozione==false){
		rimozione=true;
		t_rimozione=t; //Mi segno il tempo a cui avviene la rimozione. Devo rinormalizzare rispetto a questo
		}
    }
    
    if (coda[j].x ==svincolo && coda[j].corsia==0 && coda[j].uscita==true){ //Se la macchina si trova sullo svincolo ed è nella corsia destra e ha intenzione di uscire... Viene cancellata dall'array
      if (coda[j].v==1 || coda[j].v==0 ){
	coda.erase( coda.begin() + j );
	}
	else{ 
	  cout<<"ERRORE VELOCITA SVINCOLO"<<endl;
	}
    }
  velocita_media=velocita_media+coda[j].v;
  distanza_media=distanza_media+dist_min;
  
  if(graphics){
	 TMarker *m = new TMarker(coda[j].x,coda[j].corsia,20);
         m->SetMarkerColor(((coda[j].numero) % 4 )+1);
         m->SetMarkerSize(3);
	 m->SetMarkerStyle(((coda[j].numero)%25)+1);
	 g->Add(m,t);
   
  //Stampo a schermo
    h2.Fill(t,coda[j].v);
    h3.Fill(t,dist_min);
  }
   
     
  
    //Scrivo sul file  ( se aperto)
    if (out.is_open()){
	out<< "j:  "<<j<< "#Macchina numero: "<<coda[j].numero<<" Posizione: "<<coda[j].x<< " Velocità: "<<coda[j].v<< " Corsia: "<<coda[j].corsia<<endl;
	}
	
	  
      } // Rifaccio tutto per tutte le macchine
 print_strada(coda,street_length);
if(rimozione){
 densita=densita+(coda.size()/double(2*street_length));
 }
 tempo_array[t]=t;
 vmedia_array[t]=(velocita_media/double(coda.size()));
 distanza_media_array[t]=(distanza_media/double(coda.size()));
 //cout<<"vm "<<vmedia_array[t]<<" dist "<<distanza_media_array[t]<<endl;
 velocita_media=0;
 distanza_media=0;
 t++;	
    } // Rifaccio tutto per tutti i tempi
    
    flux_out<<(double(densita)/double(tmax-t_rimozione))<<" "<<(double(flusso)/double(tmax-t_rimozione))<<endl;
    
    
    TGraph * gr1 = new TGraph(Int_t(tmax),tempo_array,vmedia_array);
    TGraph * gr2 = new TGraph(Int_t(tmax),tempo_array,distanza_media_array);
    
    gr1->SetTitle("Velocita media");
    gr1->GetXaxis()->SetTitle("tempo");
    gr1->GetYaxis()->SetTitle("velocita media");
    
    gr2->SetTitle("Distanza media");
    gr2->GetXaxis()->SetTitle("tempo");
    gr2->GetYaxis()->SetTitle("distanza media");
    
    
    if(graphics){ 
    c1->cd(); 
     h2.Draw("lego1");
     c2->cd();
     h3.Draw("lego1");
   if (animazione){
     c3->cd();
   g-> SetSleepTime(30);
   g->Draw();
   }
   c4->cd();
   gr1->Draw("ACP");
   c4->Update();
   c5->cd();
   gr2->Draw("ACP");
   c1->Update();
   theApp->Run();
    }
    out.close(); // Chiudo il file
    flux_out.close();
    return 0;
}
