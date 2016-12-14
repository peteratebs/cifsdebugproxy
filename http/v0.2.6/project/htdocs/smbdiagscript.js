
var ntimes=0;
var xmlhttp;
var DiagConsolexmlhttp;
var DiagAjaxUpdateperiod = 0;
var diagConsole = "diagConsole"
var diagOutput = "diagOutput"
var diagConsoleLineCount = 0;
var doing_messages=0;
var diagConsoleMaxLineCount = 200;

// Update When auto refresh is enabled update the messgage window first, then update
// console
function ClearDiagConsole()
{
  diagConsoleLineCount = 0;
  document.getElementById(diagConsole).innerHTML = "";
}
// consume the text itf it is is DIAG: or NOPE: otherwise return 1 and we put it in the output window
function PrintDiagConsole(textline)
{
  if (textline.indexOf("DIAG:")==0)
  {
     document.getElementById(diagConsole).innerHTML += textline.slice(5);
     document.getElementById(diagConsole).scrollTop = document.getElementById(diagConsole).scrollHeight;
     console.log("Got text");
     return(1);
  }
  if (textline.indexOf("NOPE:")==0)
  {
      console.log("Got nope");
      return(1);
  }
  return(0);

}

function DiagAjaxUpdate()
{
    if (!xmlhttp)
    {
        if (window.XMLHttpRequest)
        {// code for IE7+, Firefox, Chrome, Opera, Safari
            xmlhttp=new XMLHttpRequest();
        }
        else
        {// code for IE6, IE5
            xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
        }

        xmlhttp.onreadystatechange=function()
        {
//            alert('onreadystatechange state=='+xmlhttp.readyState+' status=='+xmlhttp.status);
            if (xmlhttp.readyState==4 && xmlhttp.status==200)
            {
//            alert("Content type == "+xmlhttp.getResponseHeader ("Content-Type"));;
//            alert("Content length == "+xmlhttp.getResponseHeader ("Content-Length"));
//                alert("respose == "+xmlhttp.responseText);
               var r = PrintDiagConsole(xmlhttp.responseText);
               console.log("res: " + r + " for: "+ xmlhttp.responseText.slice(0,6)+" len: "+ xmlhttp.responseText.length );
               if (r==0)
               {
                 console.log("to optput: "+xmlhttp.responseText.slice(0,6));
                 document.getElementById(diagOutput).innerHTML =  xmlhttp.responseText;
               }
            }
        }
    }
    if (doing_messages)
    {
       xmlhttp.open("GET","smbdiag_ajax_getmessages",true);
       doing_messages=0; // set to 1 for messages, not working right yet
    }
    else
    {
      var diagrequest =document.getElementById("AjaxSetVal").value;
      if (diagrequest &&  diagrequest.indexOf("SMB FIDS") != -1)
      {
         xmlhttp.open("GET","smbdiag_ajax_getfids",true);
         doing_messages=1; // set to 1 for messages, not working right yet
      }
      else if (diagrequest &&  diagrequest.indexOf("SMB TIDS") != -1)
      {
         xmlhttp.open("GET","smbdiag_ajax_getfids",true);
      }
      else
      {
         xmlhttp.open("GET","smbdiag_ajax_getdefault",true);
      }
    }
//    xmlhttp.open("GET","ajax.html?10",true);
//    alert('call send new way');
    xmlhttp.send("I%20sent%20ome%20stuff%20to%20you");
    if (DiagAjaxUpdateperiod)
    {
      setTimeout("DiagAjaxUpdate()", DiagAjaxUpdateperiod);
    }
}



function ajaxRequest(){
 var activexmodes=["Msxml2.XMLHTTP", "Microsoft.XMLHTTP"]; //activeX versions to check for in IE
 if (window.ActiveXObject){ //Test for support for ActiveXObject in IE first (as XMLHttpRequest in IE7 is broken)
  for (var i=0; i<activexmodes.length; i++){
   try{
    return new ActiveXObject(activexmodes[i]);

   }
   catch(e){
    //suppress error
   }
  }
 }
 else if (window.XMLHttpRequest) // if Mozilla, Safari etc
  return new XMLHttpRequest();
 else
  return false ;
}

var mypostrequest=new ajaxRequest();
mypostrequest.onreadystatechange=function(){
 if (mypostrequest.readyState==4){
  if (mypostrequest.status==200 || mypostrequest.status==201 || window.location.href.indexOf("http")==-1){
   // alert("Yeah :"+mypostrequest.status);
  }
//  else{
//   alert("An error has occured making the request :"+mypostrequest.status);
//  }
 }
}

function doPost()
{
var value=encodeURIComponent(document.getElementById("AjaxSetVal").value);
var parameters = "AjaxSetVal="+value;

//    mypostrequest.open("POST", "http://localhost:8080/smbdiag_ajax_command_submit", true);
    mypostrequest.open("POST", "/smbdiag_ajax_command_submit", true);
    mypostrequest.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    mypostrequest.send(parameters);
}


function diagGetCommand()
{
   document.getElementById("AjaxSetVal").value = this.innerHTML;
   DiagAjaxUpdate();
}

function doClear()
{
  document.getElementById(diagOutput).innerHTML = "";
  document.getElementById(diagConsole).innerHTML = "";
}


function diagPostCommand()
{
    document.getElementById("AjaxSetVal").value = this.innerHTML;
    doPost();
}

function dostartUpdate()
{
    setTimeout('DiagAjaxUpdate(1);', 10000);
}

function dostart()
{
var r,c,x;

  // Assign handlers by class associations.

  // smbdiagposttype -
   x = document.getElementsByClassName("smbdiagposttype");
   for (r = 0; r < x.length; r++) { x[r].onclick=diagPostCommand; }

   x = document.getElementsByClassName("smbdiaggettype");
   for (r = 0; r < x.length; r++) { x[r].onclick=diagGetCommand;}

   x = document.getElementsByClassName("donotclick");
   for (r = 0; r < x.length; r++) { x[r].onclick=null; }

   document.getElementById("clicktoclear").onclick=doClear;
}

function PeriodicDiagAjaxUpdate()
{
    if (DiagAjaxUpdateperiod == 0)
      DiagAjaxUpdateperiod = 1000;
    else
      DiagAjaxUpdateperiod = 0;
    setTimeout("DiagAjaxUpdate()", DiagAjaxUpdateperiod);

}
