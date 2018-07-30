Date.prototype.pattern=function(fmt) {
	var o = {
		"M+" : this.getMonth()+1, //月份
		"d+" : this.getDate(), //日
		"h+" : this.getHours()%12 == 0 ? 12 : this.getHours()%12, //小时
		"H+" : this.getHours(), //小时
		"m+" : this.getMinutes(), //分
		"s+" : this.getSeconds(), //秒 
		"q+" : Math.floor((this.getMonth()+3)/3), //季度 
		"S" : this.getMilliseconds() //毫秒
	};
	var week = {
		"0" : "/u65e5",
		"1" : "/u4e00",
		"2" : "/u4e8c",
		"3" : "/u4e09",
		"4" : "/u56db",
		"5" : "/u4e94",
		"6" : "/u516d"
	};         
	if(/(y+)/.test(fmt)){
		fmt=fmt.replace(RegExp.$1, (this.getFullYear()+"").substr(4 - RegExp.$1.length));
	}
	if(/(E+)/.test(fmt)){
		fmt=fmt.replace(RegExp.$1, ((RegExp.$1.length>1) ? (RegExp.$1.length>2 ? "/u661f/u671f" : "/u5468") : "")+week[this.getDay()+""]);
	}
	for(var k in o){
		if(new RegExp("("+ k +")").test(fmt)){
			fmt = fmt.replace(RegExp.$1, (RegExp.$1.length==1) ? (o[k]) : (("00"+ o[k]).substr((""+ o[k]).length)));
		}
	}
	return fmt;
}

$.ajaxSetup({
	error: function (jqXHR, textStatus, errorThrown){
		if(jqXHR && jqXHR.responseText && jqXHR.responseText.indexOf("msaShowAdminLogon") > 0){
			relogon();
		}
	}
});
function relogon(){window.top.window.location = "/msa/main.xp?Fun=msaShowAdminLogon";}

function GetQueryString(name){
	var reg = new RegExp("(^|&)"+ name +"=([^&]*)(&|$)");
	var r = window.location.search.substr(1).match(reg);
	if(r!=null) return  unescape(r[2]); 
	return null;
}

function disabledEL(selecter){$(selecter).attr("disabled","disabled");}
function enabledEL(selecter){$(selecter).removeAttr("disabled");}
function readonlyEL(selecter){$(selecter).attr("readonly","readonly");}
function unreadonlyEL(selecter){$(selecter).removeAttr("readonly");}

//————————————-设置cookie——————————————————— 
function setCookie(name, value) { 
   var today = new Date(); 
   var expires = new Date(); 
   expires.setTime(today.getTime() + 1000 * 60 * 60 * 24 * 365);   // 保存一年时间的cookie 
   var the_cookie = the_cookie+ "path=/;";
   document.cookie = name + "=" + escape(value) + "; expires=" + expires.toGMTString()+";path=/;";
} 

//————————————-获取cookie——————————————————— 
function getCookie(Name) {
	var search = Name + "="; 
	if (document.cookie.length > 0) { 
		offset = document.cookie.indexOf(search); 
		if (offset != -1) { 
			offset += search.length; 
			end = document.cookie.indexOf(";", offset); 
			if (end == -1) { 
				end = document.cookie.length; 
			} 
			return unescape(document.cookie.substring(offset, end)); 
		} else { 
			return (""); 
		} 
	} else { 
		return (""); 
	} 
}

//---------------删除cookie-----------------------
function delCookie(name){
	var exp = new Date();
	exp.setTime(exp.getTime() - 1);
	var cval=getCookie(name);
	if(cval!=null)
		document.cookie= name + "="+cval+";expires=Fri, 31 Dec 1999 23:59:59 GMT;";
}

function changeDecimal_f(x, n) {
	var f_x = parseFloat(x);
	var n10 = Math.pow( 10, n);
	if (isNaN(f_x)) {
		return '0.00';
	}
	var f_x = Math.round(x * n10) / n10;
	var s_x = f_x.toString();
	var pos_decimal = s_x.indexOf('.');
	if (pos_decimal < 0) {
		pos_decimal = s_x.length;
		s_x += '.';
	}
	while (s_x.length <= pos_decimal + 2) {
		s_x += '0';
	}
	return s_x;
}

function timeSecFmt(val){return new Date(parseInt(val) * 1000).pattern("yyyy-MM-dd HH:mm:ss");}
function speedFmtWithUint(value){ return speedFmt(value) + 'bps';}
function speedFmt(value){
	if(value > 0){
		if( value < 1024 * 1024 ){
			return changeDecimal_f(value / (1024), 2) + 'K';
		}else if(value  < 1024*1024*1024){
			return changeDecimal_f(value / (1024*1024), 2) + 'M';
		}else{
			return changeDecimal_f(value / (1024*1024*1000), 2) + 'G';
		}
	}else{
		return "0.00K";
	}
}
function flowFmt(value){
	if(value > 0){
		if( value < 1024 * 1024 ){
			return changeDecimal_f(value / (1024), 2) + 'KB';
		}else if(value  < 1024*1024*1024){
			return changeDecimal_f(value / (1024*1024), 2) + 'MB';
		}else{
			return changeDecimal_f(value / (1024*1024*1000), 2) + 'GB';
		}
	}else{
		return "0.00KB";
	}
}

function numberFmt(v){
	v = parseFloat(v);
	if(v >= 100000000){
		return changeDecimal_f(v / 100000000, 2) + '亿';
	}else if(v >= 10000){
		return changeDecimal_f(v / 10000, 2) + '万';
	}else{
		return v;
	}
}

function channelFmt(v){
	var channelData = {
		"1":"(Channel 1) 2.412GHz",
		"2":"(Channel 2) 2.417GHz",
		"3":"(Channel 3) 2.422GHz",
		"4":"(Channel 4) 2.427GHz",
		"5":"(Channel 5) 2.432GHz",
		"6":"(Channel 6) 2.437GHz",
		"7":"(Channel 7) 2.442GHz",
		"8":"(Channel 8) 2.447GHz",
		"9":"(Channel 9) 2.452GHz",
		"10":"(Channel 10) 2.457GHz",
		"11":"(Channel 11) 2.462GHz",
		 
		"36":"(Channel 36) 5.18GHz",
		"40":"(Channel 40) 5.20GHz",
		"44":"(Channel 44) 5.22GHz",
		"48":"(Channel 48) 5.24GHz",
		"52":"(Channel 52) 5.26GHz",
		"56":"(Channel 56) 5.28GHz",
		"60":"(Channel 60) 5.30GHz",
		"64":"(Channel 64) 5.32GHz",
		"149":"(Channel 149) 5.745GHz",
		"153":"(Channel 153) 5.765GHz",
		"157":"(Channel 157) 5.785GHz",
		"161":"(Channel 161) 5.805GHz",
		"165":"(Channel 165) 5.825GHz"
	};
	
	if(channelData[v]){
		return channelData[v];
	}
	return "--";
}
function singalFmt(v){
	return v + " dBm";
}

function GetMaxZindex(el){
	if(el == undefined){
		el = 'body'
	}
	var objs = $(el).children();
	var maxindex = 0;
	var zi;
	var len = objs.length;
	for(var i = 0; i < len; i++){
		zi = $(objs[i]).css('zIndex');
		if(!isNaN(zi) &&  zi > maxindex ) maxindex = zi;
	}
	return maxindex;
}

function ip2int(ip) {
	var num = 0;
	ip = ip.split(".");
	num = Number(ip[0]) * 256 * 256 * 256 + Number(ip[1]) * 256 * 256 + Number(ip[2]) * 256 + Number(ip[3]);
	num = num >>> 0;
	return num;
}
function int2ip(num) {
	var str;
	var tt = new Array();
	tt[0] = (num >>> 24) >>> 0;
	tt[1] = ((num << 8) >>> 24) >>> 0;
	tt[2] = (num << 16) >>> 24;
	tt[3] = (num << 24) >>> 24;
	str = String(tt[0]) + "." + String(tt[1]) + "." + String(tt[2]) + "." + String(tt[3]);
	return str;
}