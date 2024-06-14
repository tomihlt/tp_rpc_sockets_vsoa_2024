/* This is the IDL file -- name it as app.x*/

/*combine the arguments to be passed to the server side in a structure*/
struct str{
    string s<1024>;
};

struct strw{
    string file<1024>;
	string s<1024>;
};

program APP_PROG{
   version ADD_VERS{
       str files()=1;
	   str readFile(str)=2;
	   int writeFile(strw)=3;
	   int usuarioValido(str)=4;
   }=1;
}=0x4562877;