$.ping = function(option) {
	var ping, requestTime, responseTime ;
	var getUrl = function(url){    //保证url带http://
		var strReg="^((https|http)?://){1}"
		var re=new RegExp(strReg); 
		return re.test(url)?url:"http://"+url;
	}
	
	$.ajax({
		url: (option.urlisreal == true ? option.url : getUrl(option.url)+'/'+ (new Date()).getTime() + '.html'),  //设置一个空的ajax请求
		type: 'GET',
		dataType: 'html',
		timeout: (option.timeout == undefined ? 10000 : option.timeout),
		beforeSend : function() 
		{
			if(option.beforePing) option.beforePing();
			requestTime = new Date().getTime();
		},
		complete : function(__XMLHttpRequest, __textStatus) 
		{
			//textStatus的值：success,notmodified,nocontent,error,timeout,abort,parsererror 
			responseTime = new Date().getTime();
			ping = Math.abs(requestTime - responseTime);
			if(option.afterPing) option.afterPing(ping, __XMLHttpRequest.responseText, __XMLHttpRequest.statusText);
		}
	});
	
	if(option.interval && option.interval > 0){
		setTimeout(function(){$.ping(option)}, option.interval);
	}
};