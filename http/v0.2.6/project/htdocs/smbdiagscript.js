
var ntimes=0;
var xmlhttp;

function AjaxUpdate()
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
 //               document.getElementById("myDiv").innerHTML= "Content type : "+xmlhttp.getResponseHeader("Content-Type")+"<br>";
 //               document.getElementById("myDiv").innerHTML = document.getElementById("myDiv").innerHTML + "Content length : "+xmlhttp.getResponseHeader("Content-Length")+"<br>";
                document.getElementById("myDiv").innerHTML =  xmlhttp.responseText;
            }
        }
    }
    //     xmlhttp.open("GET","http://statincInfo.html",true); -->
    xmlhttp.open("GET","demo_ajax_getval",true);
//    xmlhttp.open("GET","ajax.html?10",true);
//    alert('call send new way');
    xmlhttp.send("I%20sent%20ome%20stuff%20to%20you");
//    alert('call send is back');
//    alert("sync respose == "+xmlhttp.responseText);
//    document.getElementById("myDiv").innerHTML=xmlhttp.responseText;
    setTimeout("AjaxUpdate()", 10000);
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

//    mypostrequest.open("POST", "http://localhost:8080/demo_ajax_command_submit", true);
    mypostrequest.open("POST", "/demo_ajax_command_submit", true);
    mypostrequest.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    mypostrequest.send(parameters);
}

function PostInput()
{
    document.getElementById("AjaxSetVal").value = this.innerHTML;
    doPost();
}

function dostartUpdate()
{
    setTimeout('AjaxUpdate(1);', 10000);
}
function dostart()
{
var r,c,x;

/*
    for (r = 0; r < document.getElementById("smbdiagiapptable").rows.length; r++)
    {
        for (c=0; c<document.getElementById("smbdiagiapptable").rows[r].cells.length; c++)
        {
            document.getElementById("smbdiagiapptable").rows[r].cells[c].onclick=PostInput;
        }
   }
*/
   x = document.getElementsByClassName("smbdiagposttype");
   for (r = 0; r < x.length; r++) {
      x[r].onclick=PostInput;
   }

   x = document.getElementsByClassName("donotclick");
   for (r = 0; r < x.length; r++) {
      x[r].onclick=null;
   }

}
