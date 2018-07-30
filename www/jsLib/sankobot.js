var all_green=0;		//1全绿				0不同状态颜色不一样
var robot_width=0;		//1车宽大于40cm		0车宽30cm
var block_width=2;		//0障碍是圆形		1障碍小方块1*1			2障碍小方块2*2			3障碍小方块3*3
var pass_limit=40000;	//轨迹中两点距离平方和 40000就是200mm 大于200mm 轨迹就不连接
function ctrl_global(){
	if(all_green==1){
		vslam_init_color="#00DB00";		//绿色  
		vslam_use_color="#00DB00";		//绿色  
		vslam_loss_color="#00DB00";		//绿色  
		vslam_good_color="#00DB00";		//绿色 
	}else{
		vslam_init_color="#CAFF70";		//淡绿色    
		vslam_use_color="#B3EE3A";		//浅绿色    
		vslam_loss_color="#00DB00";		//绿色     
		vslam_good_color="#00A600";		//深绿色  
	}	
}

var vslam_init_color;		
var vslam_use_color;		
var vslam_loss_color;		
var vslam_good_color;	
var block_fang=0; 


	


function init_canvas_size(obj,id,w,h){
	obj.width=w;obj.height=h;
	$("#"+id)[0].style.width =w;
	$("#"+id)[0].style.height=h;
	return obj.getContext("2d");
}
function init_canvas(w,h){
	canvas1=document.getElementById("canvas1"); 
	canvas2=document.getElementById("canvas2"); 
	canvas3=document.getElementById("canvas3"); 
	canvas4=document.getElementById("canvas4"); 
	canvas5=document.getElementById("canvas5"); 
	canvas6=document.getElementById("canvas6");
	canvas1_1=document.getElementById("canvas1_1"); 
	//识别的物品
	context1_1=init_canvas_size(canvas1_1,"canvas1_1",w,h);
 
	//扫地机圆盘
	context1=init_canvas_size(canvas1,"canvas1",w,h);
  	//轨迹
	context2=init_canvas_size(canvas2,"canvas2",w,h);
  	//白色网状格子线
	context3=init_canvas_size(canvas3,"canvas3",w,h);
  	//蓝色行走过区域
	context4=init_canvas_size(canvas4,"canvas4",w,h);
  	//障碍物
	context5=init_canvas_size(canvas5,"canvas5",w,h);
  	//白色背景
	context6=init_canvas_size(canvas6,"canvas6",w,h);
  	
}
function init_range(l){
	$('.range5').each(function() {
		var cls = $(this).attr('class');
		var matches = cls.split(/([a-zA-Z]+)\-([0-9]+)/g);
		var options = {
			animate: true
		};
		var elem = $(this).parent();
		elem.append('<div class="uirange"></div>');
		
		for (i in matches) {
			i = i * 1;
			if (matches[i] == 'max') {
				options.max = matches[i + 1] * 1
			}
			if (matches[i] == 'min') {
				options.min = matches[i + 1] * 1
			}
		}
		options.slide = function(event, ui) {
			elem.find('span:first').empty().append(parseInt(ui.value/5)*5);
			elem.find('input:first').val(parseInt(ui.value/5)*5);
		}
		elem.find('span:first').empty().append(elem.find('input:first').val());
		options.range = 'min';
		options.value = elem.find('input:first').val() == '' ? 0 : elem.find('input:first').val();
		elem.find('.uirange').slider(options);
		$(this).hide();
	});
	$(".ui-slider-horizontal")[0].style.width=l;//滑块长度
	$(".ui-slider-horizontal")[1].style.width=l;
		
}
function init_map_file()
{
	tii=0;
	$.ajax({
        type: "POST",
        url: "/sankobot/main.xp",
        dataType:'json',
        data:{
            'Fun':'init_map_file',
       		'a':1
        },
        success: function (data){
        	if(data.status=="ok"){
        		fast_display(data.file_num,data.fp_map_offset);
        		//var version_str=$("#version_val").html();
	        	//version_str=version_str.replace("total_point_count",data.total_point_count).replace("dis_unusual_count",data.dis_unusual_count).replace("theta_unusual_count",data.theta_unusual_count).replace("average_dis_xy",data.average_dis_xy).replace("average_theta",data.average_theta).replace("max_dis_xy",data.max_dis_xy).replace("max_theta",data.max_theta);
	        	//$("#version_val").html(version_str);
        	}else{
        		//并且在后台再次运行all.sh 1 然后单独运行navi_plan  然后你就可以看到段错误了
        		//此bug很难出现...要靠运气才能碰到.....
        		//alert("map_points.csv不存在！重大bug，立刻马上拍照，通知bruce！！此时扫地机千万不要断电！！");
        		alert("map_points.csv不存在！");
        	}
        },
        error:function(){
        	//一直到ok	
    		setTimeout(function(){
    			init_map_file();
			},1000);
        }
    });
}


function ting()
{
	if(tii<10000){
		tii = 10010;
	}else{
		tii=1;
		get_current_data();
	}
}

function fast_display(file_count,fp_map_offset_parameter)
{
	var unit_count=0;
	(function display_unit_ctrl(){
		$.when(display_unit("/slampic/json/map"+unit_count+".json")).done(function () {
			++unit_count;
			if(unit_count==file_count){
				setTimeout(function(){
					fp_map_offset=fp_map_offset_parameter;
        			get_current_data();
				},1000);	
				return ;
			}
			setTimeout(function(){
				display_unit_ctrl();
			},50);
		});
	})();
	
}
	
		
		

function display_unit(path)
{
	var ajax_unit=$.ajax({
		url:path,//json文件位置
		type: "GET",//请求方式为get
		dataType: "json", //返回数据格式为json
		async: false,
		success: function(data) {//请求成功完成后要执行的方法 
			
        	var ii = 0;
        	var x_val=0;
        	var y_val=0;
        	var yaw_val=0;
        	var block_val=0;
        	var	status_color;
        	var arr;
    		$.each(data.Results, function (i, Result) {
    			map_points_arr[map_points_count]=Result.val;
    			++map_points_count;
    			++ii;
    			arr=Result.val.split("$");
    			x_val=arr[0];y_val=arr[1];
    			yaw_val=Math.round(Number(arr[2])/100);block_val=arr[3];
    			if(arr[4]==-2||arr[4]==0||arr[4]==1){
    				status_color=vslam_init_color;
    			}else if(arr[4]==2){
    				status_color=vslam_use_color;
    			}else if(arr[4]==3){
    				status_color=vslam_loss_color;
    			}else if(arr[4]==5){
    				status_color=vslam_good_color;
    			}
    			var map_x=((x_offset+Number(x_val))/scale);
	  			var map_y=((y_offset-Number(y_val))/scale);
	  			
	  			 
	  			draw_creater(map_x,map_y,status_color,status_color);//画出红色绿色区域
	  			draw_now_status(map_x,map_y,'#ff0000',0-yaw_val);//画轮廓
	  			if(block_val!="n"){ 
	  				var black_10_ang=0;
					var black_11_ang=0;
					var black_12_ang=0;
					if(block_fang==1) {
						black_10_ang=90;
						black_11_ang=-45;
						black_12_ang=45;  
					}else{
						black_10_ang=60;
						black_11_ang=-20;
						black_12_ang=20;
					}  
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(0,1),-75,0-yaw_val);//红外1
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(1,2),-60,0-yaw_val);//红外2
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(2,3),-45,0-yaw_val);//红外3
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(3,4),-30,0-yaw_val);//红外4
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(4,5),-10,0-yaw_val);//红外5
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(5,6),10,0-yaw_val);//红外6
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(6,7),30,0-yaw_val);//红外7
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(7,8),60,0-yaw_val);//红外8
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(8,9),75,0-yaw_val);//红外9
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(9,10),black_10_ang,0-yaw_val);//红外10
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(10,11),black_11_ang,0-yaw_val);//碰撞左
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(11,12),black_12_ang,0-yaw_val);//碰撞右
		  		}
            });
            //画轨迹                 
        	ii = 0;
        	var pre_x=0;
			var pre_y=0;
			var last_time_map_x=0;
			var last_time_map_y=0;
			$.each(data.Results, function (i, Result) {
        		var	pass_map_point=0;
        		arr=Result.val.split("$");
    			x_val=arr[0];y_val=arr[1];
    			if(arr[4]==3){
    				status_color="#F75000";
    			}else {
    				status_color="#F75000";
    			}
    			var map_x=((x_offset+Number(x_val))/scale);
				var map_y=((y_offset-Number(y_val))/scale);
				if(ii!=0)
				{
					var pass_point_dis=(last_time_map_x-Number(x_val))*(last_time_map_x-Number(x_val))+(last_time_map_y-Number(y_val))*(last_time_map_y-Number(y_val));
					if(pass_point_dis>pass_limit){
						pass_map_point=1; 
						//console.log(pass_point_dis);	
					}
				}
				last_time_map_x=Number(x_val);
				last_time_map_y=Number(y_val);
        		++ii;
        		context2.lineWidth=1;
				context2.strokeStyle=status_color;
            	context2.beginPath();
    			if(ii==1)
            	{
            		pre_x=Number(map_x*unit);
					pre_y=Number(map_y*unit);
            	}
            	else
            	{

					
					if(pass_map_point==0)
					{
						context2.moveTo(pre_x,pre_y);
						context2.lineTo(Number(map_x*unit),Number(map_y*unit));
					}
            		pre_x=Number(map_x*unit);
					pre_y=Number(map_y*unit);
            	}
				context2.stroke(); 
           		context2.closePath();
            });
            
		}
	});
	return ajax_unit;
}

function get_current_data() 
{
	if(tii>10000){return;}
	$.ajax({
        type: "POST",
        url: "/sankobot/main.xp",
        dataType:'json',
        data:{
            'Fun':'add_slam_point',
            'Hz':Hz,
            'fp_map_offset':fp_map_offset,
            'a':1
        },
        success: function (data){
        	tii+=1;
        	if(data.status==-3){
        		return;//数据太小
        	}else if(data.status==-2){
        		$("#status").val("空文件");tii=0;return;
        	}
        	var ii = 0;
        	var x_val=0;
        	var y_val=0;
        	var yaw_val=0;
        	var block_val=0;
        	var arr;
        	fp_map_offset=data.fp_map_offset;
    		$.each(data.Results, function (i, Result) {
    			
    			map_points_arr[map_points_count]=Result.val;
    			++map_points_count;
    			arr=Result.val.split("$");
    			x_val=arr[0];y_val=arr[1];
    			yaw_val=Math.round(Number(arr[2])/100);block_val=arr[3];
    			var map_x=((x_offset+Number(x_val))/scale);
	  			var map_y=((y_offset-Number(y_val))/scale);
    			++ii;
    			//-2	空文件 
	        	//0		没有图像数据		
	        	//1 	没有初始化		紫色
	        	//2		vslam正常		黄色
	        	//3		跟踪丢失		红色
	        	//5 	vslam正在使用	绿色
	        	if(Number(arr[4])==-3){
	        		return;//数据太小
	        	}else if(Number(arr[4])==-2){
	        		$("#status").val("空文件");tii=0;return;
	        	}else if(Number(arr[4])==0){
	        		//$("#status").val("没有图像数据");
	        		$("#status").val("0");
	        		status_color=vslam_init_color;
	        	}else if(Number(arr[4])==1){
	        		//$("#status").val("没有初始化");
	        		$("#status").val("1");
	        		status_color=vslam_init_color;
	        	}else if(Number(arr[4])==2){
	        		//$("#status").val("vslam正常");
	        		$("#status").val("2");
	        		status_color=vslam_use_color;
	        	}else if(Number(arr[4])==3){
	        		//$("#status").val("跟踪丢失");
	        		$("#status").val("3");
	        		status_color=vslam_loss_color;
	        	}else if(Number(arr[4])==5){
	        		//$("#status").val("vslam正在使用");
	        		$("#status").val("5");
	        		status_color=vslam_good_color;
	        	} 
	  			draw_creater(map_x,map_y,status_color,status_color);//画出红色绿色区域
	  			draw_now_status(map_x,map_y,'#ff0000',0-yaw_val);//画轮廓
	  			if(block_val!="n"){  
	  				var black_10_ang=0;
					var black_11_ang=0;
					var black_12_ang=0;
					if(block_fang==1) {
						black_10_ang=90;
						black_11_ang=-45;
						black_12_ang=45;  
					}else{
						black_10_ang=60;
						black_11_ang=-20;
						black_12_ang=20;
					}    
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(0,1),-75,0-yaw_val);//红外1
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(1,2),-60,0-yaw_val);//红外2
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(2,3),-45,0-yaw_val);//红外3
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(3,4),-30,0-yaw_val);//红外4
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(4,5),-10,0-yaw_val);//红外5
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(5,6),10,0-yaw_val);//红外6
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(6,7),30,0-yaw_val);//红外7
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(7,8),60,0-yaw_val);//红外8
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(8,9),75,0-yaw_val);//红外9
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(9,10),black_10_ang,0-yaw_val);//红外10
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(10,11),black_11_ang,0-yaw_val);//碰撞左
		  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(11,12),black_12_ang,0-yaw_val);//碰撞右
		  		}
		  		if(i==1){
	  				$("#slam_x").val(Math.round(x_val*100)/100);                   
                	$("#slam_y").val(Math.round(y_val*100)/100);                   
                	$("#theta").val(Number(arr[2])/100);
                	$("#action").val(data.action);                   
                	$("#mode").val(data.mode);                   
                	$("#step").val(data.step);
                	
            	}
            });
            //画轨迹                 
        	
        	ii_count=ii;
			ii = 0;
			var pre_x=0;
			var pre_y=0;
			var last_time_map_x=0;
			var last_time_map_y=0;
			$.each(data.Results, function (i, Result) {
    			var	pass_map_point=0;
    			arr=Result.val.split("$");
    			x_val=arr[0];y_val=arr[1];
    			var map_x=((x_offset+Number(x_val))/scale);
				var map_y=((y_offset-Number(y_val))/scale);
				if(ii!=0){
					var pass_point_dis=(last_time_map_x-Number(x_val))*(last_time_map_x-Number(x_val))+(last_time_map_y-Number(y_val))*(last_time_map_y-Number(y_val));
					if(pass_point_dis>pass_limit){
						//console.log(pass_point_dis);
						pass_map_point=1; 
					}
					
				}
				last_time_map_x=Number(x_val);
				last_time_map_y=Number(y_val);
				context2.lineWidth=1;
	        	if(Number(arr[4])==3){
	        		context2.strokeStyle="#F75000";
	        	}else{
	        		context2.strokeStyle="#F75000";
	        	}
				context2.beginPath();
				++ii;
				if(ii==1)
            	{
            		pre_x=Number(map_x*unit);
					pre_y=Number(map_y*unit);
            	}
            	else
            	{
					if(pass_map_point==0)
					{
						context2.moveTo(pre_x,pre_y);
						context2.lineTo(Number(map_x*unit),Number(map_y*unit));
					}
            		pre_x=Number(map_x*unit);
					pre_y=Number(map_y*unit);
            	}
				context2.stroke(); 
           		context2.closePath();
            });
           
    	},
    	error:function(){
        	tii=0;
        }
    }); 
    if(all_time_pause==0){
    	all_time_count+=(ii_count >= Hz?0.3:1.0);
    	$("#alltimecount").val(all_time_count+"s");
    }
    //console.log(all_time_count);
    setTimeout("get_current_data()",ii_count >= Hz?300:1000);	
}

function zoom_unit(){
	//tii = 10010;
	context1.clearRect(0,0,canvas1.width,canvas1.height);
	context2.clearRect(0,0,canvas2.width,canvas2.height);
	context4.clearRect(0,0,canvas4.width,canvas4.height);
	context5.clearRect(0,0,canvas5.width,canvas5.height);
	
	var x_val=0;
	var y_val=0;
	var block_val=0;
	var	status_color;
	var arr;
	var zoom_count=0;
	var pre_x=0;
	var pre_y=0;
	var last_time_map_x=0;
	var last_time_map_y=0;

			
	for(;zoom_count<map_points_count;++zoom_count){
		arr=map_points_arr[zoom_count].split("$");
		x_val=arr[0];y_val=arr[1];
		yaw_val=Math.round(Number(arr[2])/100);block_val=arr[3];
		if(arr[4]==-2||arr[4]==0||arr[4]==1){
			status_color=vslam_init_color;
		}else if(arr[4]==2){
			status_color=vslam_use_color;
		}else if(arr[4]==3){
			status_color=vslam_loss_color;
		}else if(arr[4]==5){
			status_color=vslam_good_color;
		}
		var map_x=((x_offset+Number(x_val))/scale);
		var map_y=((y_offset-Number(y_val))/scale);
			
		 
		draw_creater(map_x,map_y,status_color,status_color);//画出红色绿色区域
		if(zoom_count==map_points_count-1){
			draw_now_status(map_x,map_y,'#ff0000',0-yaw_val);//画轮廓	
		}
		if(block_val!="n"){  
			var black_10_ang=0;
			var black_11_ang=0;
			var black_12_ang=0;
			if(block_fang==1) {
				black_10_ang=90;
				black_11_ang=-45;
				black_12_ang=45;  
			}else{
				black_10_ang=60;
				black_11_ang=-20;
				black_12_ang=20;
			}
  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(0,1),-75,0-yaw_val);//红外1
  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(1,2),-60,0-yaw_val);//红外2
  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(2,3),-45,0-yaw_val);//红外3
  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(3,4),-30,0-yaw_val);//红外4
  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(4,5),-10,0-yaw_val);//红外5
  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(5,6),10,0-yaw_val);//红外6
  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(6,7),30,0-yaw_val);//红外7
  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(7,8),60,0-yaw_val);//红外8
  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(8,9),75,0-yaw_val);//红外9
  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(9,10),black_10_ang,0-yaw_val);//红外10
  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(10,11),black_11_ang,0-yaw_val);//碰撞左
  			draw_wall(map_x,map_y,'#272727','#000000',block_val.substring(11,12),black_12_ang,0-yaw_val);//碰撞右
  		}
  		//画轨迹 
		var line_color_zoom;
		if(arr[4]==3){
			line_color_zoom="#F75000";
		}else {
			line_color_zoom="#F75000";
		}
		context2.lineWidth=1;
		context2.strokeStyle=line_color_zoom;
		context2.beginPath();
		var	pass_map_point=0;
		if(zoom_count==0)
    	{
    		pre_x=Number(map_x*unit);
			pre_y=Number(map_y*unit);
    	}
    	else
    	{
			var pass_point_dis=(last_time_map_x-Number(x_val))*(last_time_map_x-Number(x_val))+(last_time_map_y-Number(y_val))*(last_time_map_y-Number(y_val));
			last_time_map_x=Number(x_val);
			last_time_map_y=Number(y_val);
			if(pass_point_dis>pass_limit){
				console.log(pass_point_dis);
				pass_map_point=1; 
			}
			if(pass_map_point==0)
			{
				context2.moveTo(pre_x,pre_y);
    			context2.lineTo(Number(map_x*unit),Number(map_y*unit));
			}
    		pre_x=Number(map_x*unit);
			pre_y=Number(map_y*unit);
    	}
		context2.stroke(); 
   		context2.closePath();
    }
    	
    
	//setTimeout(function(){
		//tii=1;
		//get_current_data();
	//},1000);
}

function ctrl_cleanner_robot(ctrl_mode,line_speed,radius)
{	
	$.ajax({
        type: "POST",
        url: "/sankobot/main.xp",
        dataType:'json',
        data:{
            'Fun':'ctrl_cleanner_robot',
            'which_robot':2,
            'ctrl_mode':ctrl_mode,
            'line_speed':Math.round(line_speed),
            'radius':Math.round(radius),
            'a':1
        },
        success: function (data){
            
        },
        error:function(){
        	
        }
    }); 	
}
function play_video(){
	show_img();
	show_door();
	setTimeout("play_video()",1000);	
}


function show_door(){
	$.ajax({
        type: "POST",
        url: "/sankobot/main.xp",
        dataType:'json',
        data:{
            'Fun':'show_door',
            'fp_item_offset':fp_item_offset,
            'a':1
        },
        success: function (data){
        	if(data.status==-3){
        		console.log('no data item update.');
        		return;//数据太小
        	}else if(data.status==-2){
        		//console.log('没有/tmp/item.csv');
        		return;
        	}
        	var arr;
        	var x_val;
        	var y_val;
        	var item_id;
        	//fp_item_offset=data.fp_item_offset;
        	fp_item_offset=0;
        	context1_1.clearRect(0,0,canvas1.width,canvas1.height);
        	$.each(data.Results, function (i, Result) {
    			arr=Result.val.split("$");
    			x_val=Math.round((x_offset+Number(arr[0]))/scale);
	  			y_val=Math.round((y_offset-Number(arr[1]))/scale);
    			item_id=arr[3];
    			draw_item_in_canvas(x_val,y_val,item_id);
    		});
        	
        },
        error:function(){
        	
        }
    }); 	
}

function draw_item_in_canvas(x,y,id)
{

    if(id==1){
    	draw_a_bottle(x,y);
    }else if(id==2){
    	draw_a_chair(x,y);
    }else if(id==3){
    	draw_a_sofa(x,y);
    }else if(id==4){
    	draw_a_tree(x,y);
    }else if(id==5){
    	draw_a_tv(x,y);
    }else if(id==6){
    	draw_a_desk(x,y);
    }else if(id==7){
    	draw_a_door(x,y);
    }
    
}  

function draw_a_door(x,y){
	
	context1_1.beginPath();
    context1_1.lineWidth=unit*radius/5;
    context1_1.strokeStyle = "#796400";
    context1_1.moveTo(unit*x-unit*radius*0.3,unit*y-unit*radius*1.1);
    context1_1.lineTo(unit*x+unit*radius*1.4,unit*y-unit*radius*1.1);
    context1_1.lineTo(unit*x+unit*radius*1.4,unit*y+unit*radius*1.1);
    context1_1.lineTo(unit*x-unit*radius*0.3,unit*y+unit*radius*1.1);
    context1_1.stroke(); 
    context1_1.closePath();
	
	context1_1.beginPath();
    context1_1.strokeStyle = "#000000";
    context1_1.fillStyle = "#C6A300";
    context1_1.moveTo(unit*x,unit*y-unit*radius);
    context1_1.lineTo(unit*x-unit*radius,unit*y-unit*radius*1.2);
    context1_1.lineTo(unit*x-unit*radius,unit*y+unit*radius*1.2);
    context1_1.lineTo(unit*x,unit*y+unit*radius);
    context1_1.lineTo(unit*x,unit*y-unit*radius);
    context1_1.stroke(); 
    context1_1.fill();
    context1_1.closePath();
	
	context1_1.fillStyle = "#796400";
 	context1_1.fillRect(unit*x-unit*radius*1.2,unit*y-unit*radius*1.2,unit*radius*0.3,unit*radius*2.4);
    
    context1_1.beginPath();
    context1_1.lineWidth=1;
    context1_1.strokeStyle = "#000000";
    context1_1.arc(unit*x-unit*radius*0.7,unit*y,radius*0.5,0,2*Math.PI,true);
    context1_1.stroke();
    context1_1.closePath();
	
}
function draw_a_tree(x,y){
	context1_1.beginPath();
    context1_1.strokeStyle = "#00A600";
    context1_1.fillStyle = "#00DB00";
    context1_1.moveTo(unit*x-unit*radius,unit*y-unit*radius);
    context1_1.lineTo(unit*x+unit*radius,unit*y-unit*radius);
    context1_1.lineTo(unit*x,unit*y-unit*radius*2);
    context1_1.lineTo(unit*x-unit*radius,unit*y-unit*radius);
    context1_1.fill();
    context1_1.closePath();
    
    context1_1.beginPath();
    context1_1.strokeStyle = "#00A600";
    context1_1.fillStyle = "#00DB00";
    context1_1.moveTo(unit*x-unit*radius*1.5,unit*y);
    context1_1.lineTo(unit*x+unit*radius*1.5,unit*y);
    context1_1.lineTo(unit*x,unit*y-unit*radius*1.5);
    context1_1.lineTo(unit*x-unit*radius*1.5,unit*y);
    context1_1.fill();
    context1_1.closePath();
    
    context1_1.fillStyle = "#00DB00";
 	context1_1.fillRect(unit*x-unit*radius*0.25,unit*y,unit*radius*0.5,unit*radius);
    
    
    context1_1.beginPath();
    context1_1.strokeStyle = "#000000";
    context1_1.moveTo(unit*x-unit*radius,unit*y-unit*radius);
    context1_1.lineTo(unit*x,unit*y-unit*radius*2); 
    context1_1.lineTo(unit*x+unit*radius,unit*y-unit*radius);
    context1_1.lineTo(unit*x+unit*radius*0.5,unit*y-unit*radius);
    context1_1.lineTo(unit*x+unit*radius*1.5,unit*y);
    context1_1.lineTo(unit*x+unit*radius*0.25,unit*y);
    context1_1.lineTo(unit*x+unit*radius*0.25,unit*y+unit*radius);
    context1_1.lineTo(unit*x-unit*radius*0.25,unit*y+unit*radius);
    context1_1.lineTo(unit*x-unit*radius*0.25,unit*y);
    context1_1.lineTo(unit*x-unit*radius*1.5,unit*y);
    context1_1.lineTo(unit*x-unit*radius*0.5,unit*y-unit*radius);
    context1_1.lineTo(unit*x-unit*radius,unit*y-unit*radius);
    context1_1.stroke(); 
    context1_1.closePath();
}
function draw_a_tv(x,y){
	
	context1_1.fillStyle = "#000000";
 	context1_1.fillRect(unit*x-unit*radius*2,unit*y-unit*radius*2,unit*radius*4,unit*radius*2);
    
    context1_1.fillStyle = "#6C6C6C";
 	context1_1.fillRect(unit*x-unit*radius*2+2,unit*y-unit*radius*2+2,unit*radius*4-4,unit*radius*2-4);
    
    context1_1.fillStyle = "#000000";
 	context1_1.fillRect(unit*x-2,unit*y,4,3);
    
    context1_1.fillStyle = "#000000";
 	context1_1.fillRect(unit*x-8,unit*y+3,16,3);
    
	
}

function draw_a_bottle(x,y){
	
    context1_1.beginPath();
    context1_1.strokeStyle = "#000000";
    context1_1.fillStyle = "#80FFFF";
    context1_1.moveTo(unit*x-unit*radius*0.5,unit*y-unit*radius);
    context1_1.lineTo(unit*x-unit*radius*0.5,unit*y+unit*radius);
    context1_1.lineTo(unit*x+unit*radius*0.5,unit*y+unit*radius);
    context1_1.lineTo(unit*x+unit*radius*0.5,unit*y-unit*radius);
    context1_1.lineTo(unit*x+unit*radius*0.5-unit*radius/3,unit*y-unit*radius-unit*radius/3);
    context1_1.lineTo(unit*x+unit*radius*0.5-unit*radius/3,unit*y-unit*radius-unit*radius);
    context1_1.lineTo(unit*x+unit*radius*0.5-2*unit*radius/3,unit*y-unit*radius-unit*radius);
    context1_1.lineTo(unit*x+unit*radius*0.5-2*unit*radius/3,unit*y-unit*radius-unit*radius/3);
    context1_1.lineTo(unit*x-unit*radius*0.5,unit*y-unit*radius);
    context1_1.stroke(); 
    context1_1.fill();
    context1_1.closePath();
    
	
}

function draw_a_chair(x,y){
	var s=3;
    context1_1.beginPath();
    context1_1.strokeStyle = "#000000";
    context1_1.fillStyle = "#796400";
    context1_1.moveTo(unit*x-unit*radius*0.75,unit*y);
    context1_1.lineTo(unit*x+unit*radius*0.75,unit*y);
    context1_1.lineTo(unit*x+unit*radius*0.75,unit*y+unit*radius*1.5);
    context1_1.lineTo(unit*x+unit*radius*0.75+s,unit*y+unit*radius*1.5);
    context1_1.lineTo(unit*x+unit*radius*0.75+s,unit*y-s);
    context1_1.lineTo(unit*x-unit*radius*0.75,unit*y-s);
    context1_1.lineTo(unit*x-unit*radius*0.75,unit*y-unit*radius*1.5);
    context1_1.lineTo(unit*x-unit*radius*0.75-s,unit*y-unit*radius*1.5);
    context1_1.lineTo(unit*x-unit*radius*0.75-s,unit*y+unit*radius*1.5);
    context1_1.lineTo(unit*x-unit*radius*0.75,unit*y+unit*radius*1.5);
    context1_1.lineTo(unit*x-unit*radius*0.75,unit*y);
    context1_1.stroke(); 
    context1_1.fill();
    context1_1.closePath();
	
	
}

function draw_a_sofa(x,y){
	context1_1.beginPath();
    context1_1.strokeStyle = "#000000";
    context1_1.fillStyle = "#796400";
    context1_1.moveTo(unit*x-unit*radius,unit*y-unit*radius);
    context1_1.lineTo(unit*x+unit*radius,unit*y-unit*radius);
    context1_1.lineTo(unit*x+unit*radius,unit*y);
    context1_1.lineTo(unit*x-unit*radius,unit*y);
    context1_1.lineTo(unit*x-unit*radius,unit*y-unit*radius);
    context1_1.stroke(); 
    context1_1.fill();
    context1_1.closePath();
    
    context1_1.beginPath();
    context1_1.strokeStyle = "#000000";
    context1_1.fillStyle = "#796400";
    context1_1.moveTo(unit*x-unit*radius,unit*y);
    context1_1.lineTo(unit*x-unit*radius*1.5,unit*y+unit*radius);
    context1_1.lineTo(unit*x-unit*radius*1.5,unit*y);
    context1_1.lineTo(unit*x-unit*radius,unit*y-unit*radius);
    context1_1.lineTo(unit*x-unit*radius,unit*y);
    context1_1.stroke(); 
    context1_1.fill();
    context1_1.closePath();
    
    context1_1.beginPath();
    context1_1.strokeStyle = "#000000";
    context1_1.fillStyle = "#796400";
    context1_1.moveTo(unit*x+unit*radius,unit*y);
    context1_1.lineTo(unit*x+unit*radius*1.5,unit*y+unit*radius);
    context1_1.lineTo(unit*x+unit*radius*1.5,unit*y);
    context1_1.lineTo(unit*x+unit*radius,unit*y-unit*radius);
    context1_1.lineTo(unit*x+unit*radius,unit*y);
    context1_1.stroke(); 
    context1_1.fill();
    context1_1.closePath();
    
    context1_1.beginPath();
    context1_1.strokeStyle = "#000000";
    context1_1.fillStyle = "#C6A300";
    context1_1.moveTo(unit*x-unit*radius,unit*y);
    context1_1.lineTo(unit*x+unit*radius,unit*y);
    context1_1.lineTo(unit*x+unit*radius*1.5,unit*y+unit*radius);
    context1_1.lineTo(unit*x-unit*radius*1.5,unit*y+unit*radius);
    context1_1.lineTo(unit*x-unit*radius,unit*y);
    context1_1.stroke(); 
    context1_1.fill();
    context1_1.closePath();
	
	context1_1.beginPath();
    context1_1.strokeStyle = "#000000";
    context1_1.fillStyle = "#796400";
    context1_1.moveTo(unit*x-unit*radius*1.5,unit*y+unit*radius);
    context1_1.lineTo(unit*x+unit*radius*1.5,unit*y+unit*radius);
    context1_1.lineTo(unit*x+unit*radius*1.5,unit*y+unit*radius*1.3);
    context1_1.lineTo(unit*x-unit*radius*1.5,unit*y+unit*radius*1.3);
    context1_1.lineTo(unit*x-unit*radius*1.5,unit*y+unit*radius); 
    context1_1.stroke(); 
    context1_1.fill();
    context1_1.closePath();
	
	context1_1.fillStyle = "#000000";
 	context1_1.fillRect(unit*x-unit*radius,unit*y+unit*radius*1.3,unit*radius*0.2,unit*radius*0.2);
    
    context1_1.fillStyle = "#000000";
 	context1_1.fillRect(unit*x+unit*radius*0.8,unit*y+unit*radius*1.3,unit*radius*0.2,unit*radius*0.2);
    
}

function draw_a_desk(x,y){
	
	context1_1.fillStyle = "#000000";
 	context1_1.fillRect(unit*x-unit*radius*1.5,unit*y,unit*radius*0.2,unit*radius);
	
	context1_1.fillStyle = "#000000";
 	context1_1.fillRect(unit*x+unit*radius*0.3,unit*y,unit*radius*0.2,unit*radius);
	
	context1_1.fillStyle = "#000000";
 	context1_1.fillRect(unit*x-unit*radius*0.5,unit*y,unit*radius*0.2,unit*radius*0.5);
	
	context1_1.fillStyle = "#000000";
 	context1_1.fillRect(unit*x+unit*radius*1.3,unit*y-unit*radius,unit*radius*0.2,unit*radius*1.5);
	
	
	context1_1.beginPath();
    context1_1.strokeStyle = "#000000";
    context1_1.fillStyle = "#796400";
    context1_1.moveTo(unit*x-unit*radius*0.5,unit*y-unit*radius);
    context1_1.lineTo(unit*x+unit*radius*1.5,unit*y-unit*radius);
    context1_1.lineTo(unit*x+unit*radius*0.5,unit*y);
    context1_1.lineTo(unit*x-unit*radius*1.5,unit*y);
    context1_1.lineTo(unit*x-unit*radius*0.5,unit*y-unit*radius);
    context1_1.stroke(); 
    context1_1.fill();
    context1_1.closePath();
	
	
}


function show_img(){
	$.ajax({
        type: "POST",
        url: "/sankobot/main.xp",
        dataType:'json',
        data:{
            'Fun':'play_video',
            'a':1
        },
        success: function (data){
        	if(data.path=="null"||data.map=="null"){
        		return;
        	}else{
        		$("#play_img")[0].src=data.path;
        		$("#play_map")[0].src=data.map;
        		
        	}
        	
        },
        error:function(){
        	
        }
    }); 
}


function get_2nd_img(obj){
	obj.onerror=null;
	$.ajax({
        type: "POST",
        url: "/sankobot/main.xp",
        dataType:'json',
        async:false,
        data:{
            'Fun':'play_video',
            'a':1
        },
        success: function (data){
        	console.log(data.path);
        	$("#play_img")[0].src=data.path;
        },
        error:function(){
        	
        }
    }); 
}
function get_config_val(path,name){
	var configVal="unknow";
	$.ajax({
        type: "POST",
        url: "/sankobot/main.xp",
        dataType:'json',
        data:{
            'Fun':'get_config_val',
            'name':name,
            'path':path,
            'a':1
        },
        success: function (data){
        	configVal = data.val;
        	var version_str=$("#version_val").html();
        	version_str=version_str.replace(name,configVal);
        	$("#version_val").html(version_str);
        },
        error:function(){
        	
        }
    });
}

//画出行走区域（绿色和红色）
function draw_creater(x,y,c1,c2)
{
	var s=1;
	if(robot_width==1){
		s=1.4;
	}else{
		s=1;	
	}
	context4.beginPath();
    context4.strokeStyle = c1;
    context4.arc(unit*x,unit*y,unit*radius*s,0,2*Math.PI,true); 
    context4.closePath();
    context4.fillStyle = c2;
    context4.fill(); 
}

//扫地机圆盘
function draw_now_status(x,y,c1,ang)
{
	var s=1;
	if(robot_width==1){
		s=1.3;
	}else{
		s=1;	
	}
	context1.clearRect(0,0,canvas1.width,canvas1.height);
	var head_x=Math.round(Number(x)+(radius*s*1.1)*Math.cos(Math.PI/180*Number(ang)));
	var head_y=Math.round(Number(y)+(radius*s*1.1)*Math.sin(Math.PI/180*Number(ang)));
	context1.beginPath();
    context1.strokeStyle = c1;
    context1.fillStyle = c1;
    context1.arc(unit*x,unit*y,unit*radius*s*1.1,0,2*Math.PI,true);
    context1.fill();
    context1.closePath();
     
    context1.beginPath();
    context1.strokeStyle ='#caffff';
    context1.fillStyle ='#caffff';
    context1.arc(unit*x,unit*y,unit*radius*s*1,0,2*Math.PI,true);
    context1.fill();
    context1.closePath();
    
    context1.beginPath();
    context1.lineWidth=2;
	context1.strokeStyle="#ff8000";
    context1.moveTo(unit*x,unit*y);
	context1.lineTo(head_x*unit,head_y*unit);
    context1.stroke(); 
    context1.closePath();
}  
//障碍物和墙壁
function draw_wall(x,y,c1,c2,v,ang,dif)
{
	block_fang=0;
	var s=1;
	if(robot_width==1){
		s=1.8;
	}else{
		s=1.5;	
	}
	if(v==0)return;//没障碍物
	if(block_width==0){
		context5.beginPath();
		context5.strokeStyle = c1;
		context5.lineWidth = unit*2;
		context5.arc(unit*x,unit*y,unit*radius*s,Math.PI/180*(Number(ang)+dif)+Math.PI/8,Math.PI/180*(Number(ang)+dif)-Math.PI/8,true);
		context5.stroke();
		context5.closePath();
	}else{
		var block_x=(Number(x)+(radius+2)*Math.cos(Math.PI/180*(Number(ang)+dif)));
		var block_y=(Number(y)+(radius+2)*Math.sin(Math.PI/180*(Number(ang)+dif)));
		if(block_width==1){
			context5.fillRect(unit*block_x,unit*block_y,unit,unit);
		}else if(block_width==2){
			context5.fillRect(unit*(block_x-1),unit*(block_y-1),unit*2,unit*2);
		}else if(block_width==3){
			context5.fillRect(unit*(block_x-1.5),unit*(block_y-1.5),unit*3,unit*3);
		}
	}
}
//画出网格线
function draw_white_grid(t,n)
{
	setTimeout(function(){
		context3.lineWidth=1;
		context3.strokeStyle="#f7f7f7";
    	for(i=0;i<canvas3.height/n+1;++i)
    	{
        	context3.beginPath();
        	context3.moveTo(0,n*i);
			context3.lineTo(canvas3.width,n*i);
            context3.stroke(); 
           	context3.closePath();
    	}
    	for(i=0;i<canvas3.width/n+1;++i)
    	{
        	context3.beginPath();
        	context3.moveTo(n*i,0);
			context3.lineTo(n*i,canvas3.height);
            context3.stroke(); 
           	context3.closePath();
    	} 
	},t);	
}
function goto_ab(x,y,v)
{
	$.ajax({
	    type: "POST",
	    url: "/sankobot/main.xp",
	    dataType:'json',
	    data:{
	        'Fun':'ctrl_cleanner_robot',
	        'goto_x':x,
	        'goto_y':y,
	        'which_robot':2,
	        'ctrl_mode':10,
	        'line_speed':v,
	        'radius':0,   
	        'a':1
	    },
	    success: function (data){
	        
	    },
	    error:function(){
	    	
	    }
	}); 
}
	  	

//移动红色地图标记点
function move_mark_point(xxx,yyy,page_x,page_y){
	$("#target")[0].style.left=page_x;
	$("#target")[0].style.top=page_y;
	$("#target")[0].style.display="none";
	document.getElementById('openIframe').contentWindow.document.getElementById("goto_AB_x").name=xxx;
	document.getElementById('openIframe').contentWindow.document.getElementById("goto_AB_y").name=yyy;
	
}
function read_me(){
	$.messager.alert('Readme', "<div >控制说明：</div>"
										
										+"<table>"	
											+"<tr>"
												+"<td style='width:100px;'></td>"
												+"<td style='width:150px;'>8(↑)</td>"
												+"<td style='width:150px;'>前进</td>"
											+"</tr>"
											+"<tr>"
												+"<td></td>"
												+"<td>2(↓)</td>"
												+"<td>后退</td>"
											+"</tr>"
											+"<tr>"
												+"<td></td>"
												+"<td>4(←)</td>"
												+"<td>左旋</td>"
											+"</tr>"
											+"<tr>"
												+"<td></td>"
												+"<td>6(→)</td>"
												+"<td>右旋</td>"
											+"</tr>"
											+"<tr>"
												+"<td></td>"
												+"<td>5(空格)</td>"
												+"<td>暂停</td>"
											+"</tr>"
											+"<tr>"
												+"<td></td>"
												+"<td>A</td>"
												+"<td>(Auto)自动模式</td>"
											+"</tr>"
											+"<tr>"
												+"<td></td>"
												+"<td>D</td>"
												+"<td>(Dock)回充</td>"
											+"</tr>"
											+"<tr>"
												+"<td></td>"
												+"<td>G</td>"
												+"<td>(Go暂时没有)A-B</td>"
											+"</tr>"
											+"<tr>"
												+"<td></td>"
												+"<td>H</td>"
												+"<td>(Hide)隐藏轨迹</td>"
											+"</tr>"
											+"<tr>"
												+"<td></td>"
												+"<td>ctrl+H</td>"
												+"<td>(Hide)隐藏行走区域</td>"
											+"</tr>"
											+"<tr>"
												+"<td></td>"
												+"<td>P</td>"
												+"<td>(Point)定点清扫</td>"
											+"</tr>"
											+"<tr>"
												+"<td></td>"
												+"<td>S</td>"
												+"<td>(Stop)停车</td>"
											+"</tr>"
											+"<tr>"
												+"<td></td>"
												+"<td>V</td>"
												+"<td>(Video)可视化界面变大变小</td>"
											+"</tr>"
											+"<tr>"
												+"<td></td>"
												+"<td>W</td>"
												+"<td>(Wall)沿墙</td>"
											+"</tr>"
											+"<tr>"
												+"<td></td>"
												+"<td>ctrl+↑</td>"
												+"<td>地图上移</td>"
											+"</tr>"
											+"<tr>"
												+"<td></td>"
												+"<td>ctrl+↓</td>"
												+"<td>地图下移</td>"
											+"</tr>"
											+"<tr>"
												+"<td></td>"
												+"<td>ctrl+←</td>"
												+"<td>地图左移</td>"
											+"</tr>"
											+"<tr>"
												+"<td></td>"
												+"<td>ctrl+→</td>"
												+"<td>地图右移</td>"
											+"</tr>"
											+"<tr>"
												+"<td></td>"
												+"<td>ctrl++</td>"
												+"<td>地图放大</td>"
											+"</tr>"
											+"<tr>"
												+"<td></td>"
												+"<td>ctrl+-</td>"
												+"<td>地图缩小</td>"
											+"</tr>"
										+"</table>");
	
	$(".messager-window")[0].style.width="800px";
	$(".messager-window")[0].style.left=(window.innerWidth-800)/2+"px";
	$(".panel-header")[0].style.width="800px";
	$(".messager-body")[0].style.width="800px";
	$(".window-shadow")[0].style.display="none";
	$("div[class='panel-tool'] a")[0].style.display="none";
	$(".l-btn").click(function () {
		
	});  

}