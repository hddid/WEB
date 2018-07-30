/*****************等待中禁用页面功能*****************/  
/** 
 * 禁用页面 
 */  
function forbiddenPage(){  
    $("<div class=\"datagrid-mask\" style=\"background:#666666;\"></div>").css({display:"block",width:$("body")[0].offsetWidth+10,height:$(window).height()}).appendTo("body");   
    $("<div class=\"datagrid-mask-msg\"></div>").html("正在处理，请稍候……").appendTo("body").css({zIndex:999999, display:"block",left:($(document.body).outerWidth(true) - 190) / 2,top:($(window).height() - 45) / 2});  
}  
/** 
 * 释放页面 
 * @return 
 */  
function releasePage(){  
    $(".datagrid-mask,.datagrid-mask-msg").hide();  
}