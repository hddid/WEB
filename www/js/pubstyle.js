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
  if(document.cookie.length > 0) { 
    offset = document.cookie.indexOf(search); 
    if (offset != -1) { 
      offset += search.length; 
      end = document.cookie.indexOf(";", offset); 
      if (end == -1) { 
        end = document.cookie.length; 
      } 
      return unescape(document.cookie.substring(offset, end)); 
    }else { 
      return (""); 
    } 
  }else { 
    return (""); 
  } 
}

//---------------删除cookie-----------------------
function delCookie(name){
			document.cookie = name+"=;expires="+(new Date(0)).toGMTString();
}
