/* singer.c */
/* One sings can the other find a harmony? */

#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#define SA struct sockaddr 
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

/* here are our X variables */
Display *display;
XColor    color[100];
int screen;
Window win;
GC gc;
unsigned long black,white;
#define X_SIZE 1920 
//#define TX_SIZE 3840
#define TX_SIZE 20000 
#define Y_SIZE 1080 

/* here are our X routines declared! */
void init_x();
void close_x();
void redraw();

#define MAX 80 
#define PORT 9080 

/* sound */
void *spkr();
void *comms();
void *control();

/*static float notes[108]={
0,17.32,18.35,19.45,20.60,21.83,23.12,24.50,25.96,27.50,29.14,30.87,
32.70,34.65,36.71,38.89,41.20,43.65,46.25,49.00,51.91,55.00,58.27,61.74,
65.41,69.30,73.42,77.78,82.41,87.31,92.50,98.00,103.8,110.0,116.5,123.5,
130.8,138.6,146.8,155.6,164.8,174.6,185.0,196.0,207.7,220.0,233.1,246.9,
261.6,277.2,293.7,311.1,329.6,349.2,370.0,392.0,415.3,440.0,466.2,493.9,
523.3,554.4,587.3,622.3,659.3,698.0,740.0,784.0,830.6,880.0,932.3,987.8,
1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976,
2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951,
4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902 };  */

static float notes[11]={0,277.2,311.1,370.0,415.3,466.2,554.4,622.3,740.0,830.6,932.3};

struct output { short *waveform; long where; int lno; int rno; float t; float lpf;};

void usage ()
{
	printf("usage: singer\n");
	exit (1);
}

int main(int argc,char *argv[])
{
 
	int *fhead,len,chan,sample_rate,bits_pers,byte_rate,ba,size,seed;
	int number,along,osc,note,leg;
	char stop;
	struct output *out;

	init_x();

        out=(struct output *)malloc(sizeof(struct output ));

	seed=atoi(argv[1]);

        fhead=(int *)malloc(sizeof(int)*11);

        len=60*60; //1 hour
        chan=2;
        sample_rate=44100;
        bits_pers=16;
        byte_rate=(sample_rate*chan*bits_pers)/8;
        ba=((chan*bits_pers)/8)+bits_pers*65536;
        size=chan*len*sample_rate;
	leg=2;
	out->t=1;
	out->lpf=2;

	srand(seed);

        out->waveform=(short *)malloc(sizeof(short)*size);

	//printf ("waiting \n");
	//scanf("%c",&stop);

       pthread_t spkr_id,comms_id,control_id;

       struct timespec tim, tim2;
               tim.tv_sec = 0;
        tim.tv_nsec = 100L;

        pthread_create(&spkr_id, NULL, spkr, out);
        pthread_create(&control_id, NULL, control, out);
        //pthread_create(&comms_id, NULL, comms, out);

	long my_point,ahead,chunk,psum,dtot,avg;
	chunk=2206;
	ahead=4410;
	my_point=50*ahead;
	int vamp,lamp,ramp,lnote,llnote,rnote,mnote,lcount,tnote,minnote,maxnote,lchoice,a,lattack,rattack;
	float max,min,lp,rp,fl,fr,fd,ft,thresh,fm;

	lnote=5;lcount=0;
	rnote=6;tnote=1;
	mnote=7;
	lamp=0;ramp=0;
	lchoice=10;
	a=18820;
	lattack=2;rattack=2;
	vamp=300;

	thresh=1.01;

	fr=2*M_PI*notes[lnote]/44100;

	float mosc,losc,rosc,tosc,t,ph;
	losc=0;rosc=0;tosc=0;
	avg=0; fd=0;
	min=100000;
	max=0;
	ph=0;
	int lpp,aplus;
	float ll,rr,mm,dlpf,dt,ddt;
	ddt=0.0002;
	dt=0.000004;
	dlpf=0.00003;
	ll=0;rr=0; lpp=-5;aplus=2;
	t=1;
	while (1>0)
	{
		long trigger,diff;
		int wcount;
		trigger=my_point-ahead;
		wcount=0;
		while (out->where<trigger)
		{
                       nanosleep(&tim , &tim2);
		       wcount++;

		}
		if (wcount < 20){printf ("%d\n",wcount);}

		lcount++;
		//if (lcount>200){lcount=0;lnote++;if(lnote>59){lnote=48;}printf("%d\n",lnote);}
		//if (lcount>150){lcount=0;lnote=55+rand()%5;printf("%d\n",lnote);}
		//if (lcount%150==0){a+=aplus;vamp=0;aplus+=(10-rand()%20);
		//	if (a<8810){ a=8810;aplus=-aplus;}
		//	if (a>88200){ a=88200;aplus=-aplus;}
		//}
		thresh+=ddt;if (thresh<0){thresh=0;ddt=-ddt;}
		if (thresh>1){thresh=1;ddt=-ddt;}
		//t-=0.001;if(t<-1){t=1;}

		fl=(M_PI*(notes[lnote])/22050);
		fr=M_PI*(notes[rnote])/22050;
		ft=M_PI*(notes[tnote])/22050;
		fm=M_PI*(notes[mnote])/44100;
		long from,to,sum;
		from=my_point;
		to=my_point+chunk;
		sum=0;
		for (my_point=from;my_point<to;my_point+=2)
		{
			short left,right,test,mid;
			float leftf,rightf,midf;
			int tot;
			ph+=0.0002935;if (ph>2*M_PI){ph=0;}
			if(my_point%300==0){a+=aplus; if (a<14410){ a=14410;aplus=-aplus;} if (a>108820){ a=108820;aplus=-aplus;}}
			out->lpf+=dlpf;if(out->lpf<0){out->lpf=0;dlpf=-dlpf;}	
			if(out->lpf>2){out->lpf=2;dlpf=-dlpf;}	
			out->t+=dt;if(out->t<-1){out->t=-1;dt=-dt;}	
			if(out->t>1){out->t=1;dt=-dt;}	

			losc+=(fl+((sin(my_point*M_PI/(leg*200))/850)));
			rosc+=(fr+((sin(my_point*M_PI/(leg*205))/750)));
			mosc+=(fm);
			tosc+=ft;
			leftf=(sin(losc+ph));
			rightf=(sin(rosc-ph));
			midf=(sin(mosc));
			test=32767*(cos(tosc));

			//threshold it
			if (leftf>(thresh) ){ leftf=out->t;}
			if (leftf<-(thresh) ){ leftf=-out->t;}
			if (rightf>(thresh) ){rightf=out->t;}
			if (rightf<-(thresh) ){rightf=-out->t;}
			if (midf>(1-thresh) ){midf=1;}

			//filter it
			float dm,dl,dr,lpf;
			lpf=out->lpf;
			dl=leftf-ll;
			dr=rightf-rr;
			dm=midf-mm;
			if (dl>lpf){ leftf=ll+lpf;}
			if (-dl>lpf){ leftf=ll-lpf;}
			if (dr>lpf){ rightf=rr+lpf;}
			if (-dr>lpf){ rightf=rr-lpf;}
			if (dm>lpf){ midf=mm+lpf;}
			if (-dm>lpf){ midf=mm-lpf;}
			ll=leftf;
			rr=rightf;
			mm=midf;


			// add reverb
			left=lamp*leftf;
			right=ramp*rightf;
			mid=32767*midf;
			// verb 
			out->waveform[my_point]=(mid/7)+(left/3)+(300*out->waveform[my_point-a+1]/600);
			out->waveform[my_point+1]=(mid/7)+(right/3)+(300*out->waveform[my_point-a]/600);
			//tot=((out->waveform[my_point]+out->waveform[my_point+1])/2)+test;
			tot=((left+right)/2)+test;
			if (tot<0){tot=-tot;}
			sum=sum+tot;
			avg=avg+sum;
			if (lamp<32767 && my_point%lattack==0 ){lamp++;}
			if (ramp<32767 && my_point%rattack==0  ){ramp++;}
		}
		diff=sum-psum;
		psum=sum;
		if (diff<0){diff=-diff;}
		dtot+=diff;
		if (lcount%leg==0){
			float v;
			long reached;
			reached=out->where;
			v=((float)(4000000*dtot)/(float)avg);
			//printf ("%ld %d %f\n",dtot, tnote,v);
			dtot=0;
			avg=0;
			if (v<min){ minnote=tnote;min=v;}
			if (v>max){ maxnote=tnote;max=v;}
			tosc=0;
			dt=0.00001;
			dlpf=0.00009;
			//XSetForeground(display,gc,color[lnote%12+1].pixel);
			//XFillRectangle(display, win, gc, 70+(tnote%12)*70, 100, 60, 200);
			/*XSetForeground(display,gc,color[tnote].pixel);
			//XFillRectangle(display, win, gc, 70+(tnote%12)*140, 20+(300*((tnote-36)/12)), 130,1 5*v);
			XFillRectangle(display, win, gc, (tnote*137), (Y_SIZE-v)/2, 137, v);
			XSetForeground(display,gc,black);
			XDrawRectangle(display, win, gc, (tnote*137), (Y_SIZE-v)/2, 137, v); */
			int xd;
			XSetForeground(display,gc,color[13].pixel);
			for (xd=0;xd<TX_SIZE;xd++)
			{
			int rp,lp,yp,wp,xxx;
			wp=reached-xd-(TX_SIZE);
			lp=500+(out->waveform[wp]/58);
			rp=500+(out->waveform[wp+1]/58);
			xxx=xd/2;
			/*XDrawPoint(display, win, gc, xxx,lp+1);
			XDrawPoint(display, win, gc, xxx,lp);
			XDrawPoint(display, win, gc, xxx,lp-1);
			XDrawPoint(display, win, gc, xxx,rp+1);
			XDrawPoint(display, win, gc, xxx,rp);
			XDrawPoint(display, win, gc, xxx,rp-1)*/
			//XSetForeground(display,gc,color[13].pixel);
			XDrawPoint(display, win, gc, rp,lp);
			//XSetForeground(display,gc,color[14].pixel);
			}
			XSetForeground(display,gc,color[14].pixel);
			for (xd=0;xd<TX_SIZE;xd++)
			{
			int rp,lp,yp,wp,xxx;
			wp=reached-xd-(TX_SIZE);
			lp=(out->waveform[wp]/58);
			rp=500+(out->waveform[wp+1]/58);
			XDrawPoint(display, win, gc, 1500-lp,rp);
			}


				
			XFlush(display);
			tnote++;
			if (tnote>11){tnote=1;
				min=1000000;
				leg=1+(v/30);
				lattack=leg;
				rattack=leg;
				max=0;
				//if (minnote==lchoice){ lnote=minnote;losc=0;rnote=0;}else{ rnote=minnote;rosc=0;}
				//lnote=minnote;rnote=maxnote;
				lnote=minnote;rnote=lchoice; mnote=llnote;
				llnote=lchoice;
				lchoice=lnote;
				losc=0;rosc=0;mosc=0;
				if (rand()%100>80){lnote=0;}
				if (rand()%100>80 && lnote>0 ){rnote=0;}
				//if (minnote==lchoice){ lnote=0;}
				//if (rnote==lchoice){ rnote=48;}
				out->lpf=0;
				out->t=-1.0;
				printf ("chosing l %d r %d a %d thresh %f lpf %f leg %d t %f ph %f\n",lnote,rnote,a,thresh,out->lpf,leg,out->t,ph);
				out->lno=lnote;out->rno=rnote;
				XSetForeground(display,gc,color[lnote].pixel);
				XFillRectangle(display, win, gc, 0, 0, X_SIZE/2, Y_SIZE);
				XSetForeground(display,gc,color[rnote].pixel);
				XFillRectangle(display, win, gc, X_SIZE/2, 0, X_SIZE/2, Y_SIZE); 
			}
			//XFillRectangle(display, win, gc, 0, 0, X_SIZE/2, Y_SIZE);
			//lattack++;
			//if (lattack>20){lattack=1;}
			//rattack=21-lattack;
		}
	} 
}



void *control(void *o) {
        // This handles the speakers.

  struct output *out;
  out=(struct output *)o;
  char c;

	while (1>0)
	{
		scanf ("%c",&c);
		if (c == 'r')
		{
			printf ("%c\n",c);

        int *fhead,chan,sample_rate,bits_pers,byte_rate,ba,size;
        fhead=(int *)malloc(sizeof(int)*11);

        chan=2;
        sample_rate=44100;
        bits_pers=16;
        byte_rate=(sample_rate*chan*bits_pers)/8;
        ba=((chan*bits_pers)/8)+bits_pers*65536;
        size=out->where;

        fhead[0]=0x46464952;
        fhead[1]=36;
        fhead[2]=0x45564157;
        fhead[3]=0x20746d66;
        fhead[4]=16;
        fhead[5]=65536*chan+1;
        fhead[6]=sample_rate;
        fhead[7]=byte_rate;
        fhead[8]=ba;
        fhead[9]=0x61746164;
        fhead[10]=(size*chan*bits_pers)/8;


        FILE *record;
        record=fopen("record1.wav","wb");
        fwrite(fhead,sizeof(int),11,record);
        fwrite(out->waveform,sizeof(short),out->where,record);
        fclose (record);
        free(fhead);
	}
		if (c == 't'){ printf ("Gimmie t \n"); scanf("%f",&out->t); }
		if (c == 'f'){ printf ("Gimmie f \n"); scanf("%f",&out->lpf); }
	}
}



void *spkr(void *o) {
        // This handles the speakers.

  struct output *out;
  int rc;
  int size;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  unsigned int val;
  int dir;
  snd_pcm_uframes_t frames;

  out=(struct output *)o;

  //char *buffer;

  /* Open PCM device for playback. */
  rc = snd_pcm_open(&handle, "default",
                    SND_PCM_STREAM_PLAYBACK, 0);
  if (rc < 0) {
    fprintf(stderr,
            "unable to open pcm device: %s\n",
            snd_strerror(rc));
    exit(1);
  }
  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);
  /* Fill it in with default values. */
  snd_pcm_hw_params_any(handle, params);
  /* Set the desired hardware parameters. */
  /* Interleaved mode */
  snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
  /* Two channels (stereo) */
  snd_pcm_hw_params_set_channels(handle, params, 2);
  /* 44100 bits/second sampling rate (CD quality) */
  val = 44100;
  snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
  /* Set period size to 32 frames. */
  frames = 64;
  snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
  /* Write the parameters to the aux */
  rc = snd_pcm_hw_params(handle, params);
  if (rc < 0) {
    fprintf(stderr,
            "unable to set hw parameters: %s\n",
            snd_strerror(rc));
    exit(1);
  }

  /* Use a buffer large enough to hold one period */
  snd_pcm_hw_params_get_period_size(params, &frames, &dir);
  //size = frames * 4; /* 2 bytes/sample, 2 channels 
  // size as in number of data points along
  size = frames * 2;

  snd_pcm_hw_params_get_period_time(params, &val, &dir);

  while (1 > 0) {
    rc = snd_pcm_writei(handle, (out->waveform+out->where), frames);
    if (rc == -EPIPE) {
      /* EPIPE means underrun */
      fprintf(stderr, "underrun occurred\n");
      snd_pcm_prepare(handle);
    } else if (rc < 0) {
      fprintf(stderr,
              "error from writei: %s\n",
              snd_strerror(rc));
    }  else if (rc != (int)frames) {
      fprintf(stderr,
              "short write, write %d frames\n", rc);
    }
    out->where+=size;
  }

  snd_pcm_drain(handle);
  snd_pcm_close(handle);
  //free(buffer);

  return 0;
}

void init_x()
{
/* get the colors black and white (see section for details) */
        XInitThreads();
        //x_buffer=(unsigned char *)malloc(sizeof(unsigned char)*4*VX_SIZE*VY_SIZE);
        display=XOpenDisplay((char *)0);
        screen=DefaultScreen(display);
        black=BlackPixel(display,screen),
        white=WhitePixel(display,screen);
        win=XCreateSimpleWindow(display,DefaultRootWindow(display),0,0, X_SIZE, Y_SIZE, 5, white,black);
        XSetStandardProperties(display,win,"PC scope","PC scope",None,NULL,0,NULL);
        XSelectInput(display, win, ExposureMask|ButtonPressMask|KeyPressMask|ButtonReleaseMask|ButtonMotionMask);
        //XSelectInput(display, vwin, ExposureMask|ButtonPressMask|KeyPressMask|ButtonReleaseMask|ButtonMotionMask);
        gc=XCreateGC(display, win, 0,0);
        XSetBackground(display,gc,black); XSetForeground(display,gc,white);
        XClearWindow(display, win); 
        XMapRaised(display, win);
        XMoveWindow(display, win,0,0);
        Visual *visual=DefaultVisual(display, 0);
        Colormap cmap;
        cmap = DefaultColormap(display, screen);
        color[0].red = 65535; color[0].green = 65535; color[0].blue = 65535;

        color[1].red = 65535; color[1].green = 0; color[1].blue = 0;
        color[2].red = 0; color[2].green = 65535; color[2].blue = 0;
        color[3].red = 0; color[3].green = 0; color[3].blue = 65535;

        color[4].red = 0; color[4].green = 65535; color[4].blue = 65535;
        color[5].red = 65535; color[5].green = 65535; color[5].blue = 0;
        color[6].red = 65535; color[6].green = 0; color[6].blue = 65535;

        color[7].red = 32768; color[7].green = 65535; color[7].blue = 0;
        color[8].red = 65535; color[8].green = 32768; color[8].blue = 0;
        color[9].red = 0; color[9].green = 65535; color[9].blue = 32768;

        color[10].red = 65535; color[10].green = 65535; color[10].blue = 32768;
        color[11].red = 65535; color[11].green = 32768; color[11].blue = 65535;
        color[12].red = 32768; color[12].green = 65535; color[12].blue = 65535;

        color[13].red = 32768; color[13].green = 0; color[13].blue = 0;
        color[14].red = 0; color[14].green = 32768 ; color[14].blue = 0;

        XAllocColor(display, cmap, &color[0]);
        XAllocColor(display, cmap, &color[1]);
        XAllocColor(display, cmap, &color[2]);
        XAllocColor(display, cmap, &color[3]);
        XAllocColor(display, cmap, &color[4]);
        XAllocColor(display, cmap, &color[5]);
        XAllocColor(display, cmap, &color[6]);
        XAllocColor(display, cmap, &color[7]);
        XAllocColor(display, cmap, &color[8]);
        XAllocColor(display, cmap, &color[9]);
        XAllocColor(display, cmap, &color[10]);
        XAllocColor(display, cmap, &color[11]);
        XAllocColor(display, cmap, &color[12]);
        XAllocColor(display, cmap, &color[13]);
        XAllocColor(display, cmap, &color[14]);
}

