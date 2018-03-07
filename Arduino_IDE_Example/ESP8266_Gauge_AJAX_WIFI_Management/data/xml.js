status=0;
xmlHttp=new XMLHttpRequest();
function process(){
 if(xmlHttp.readyState==0 || xmlHttp.readyState==4){
   xmlHttp.open('PUT','xml',true);
   xmlHttp.onreadystatechange=handleServerResponse; // no brackets?????
   xmlHttp.send(null);
 }
 setTimeout('process()',10000);
}
function handleServerResponse(){
 if(xmlHttp.readyState==4 && xmlHttp.status==200){
   xmlResponse=xmlHttp.responseXML;
   //xmldoc = xmlResponse.getElementsByTagName('temperature');
   //message = xmldoc[0].firstChild.nodeValue;
   message = xmlResponse.getElementsByTagName('temperature')[0].childNodes[0].nodeValue;
   message2 = xmlResponse.getElementsByTagName('humidity')[0].childNodes[0].nodeValue;
   message3 = xmlResponse.getElementsByTagName('alert')[0].childNodes[0].nodeValue;
   if(message3 == 0){
      alert('Sensor Failure Detected');
      status = 1;
   }
   else if(message3*status){
      alert('Sensor is back online');
      status = 0;
   }  
   Gauge.Collection.get('gauge1').setValue(message);
   Gauge.Collection.get('gauge2').setValue(message2);
   //xmldoc = xmlResponse.getElementsByTagName('Humidity');
   //message2 = xmldoc[0].firstChild.nodeValue;
   //document.getElementById('runtime').innerHTML=message;
   //Gauge.Collection.get('gauge2').setValue(message2);
 }
}
