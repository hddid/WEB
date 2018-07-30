	/*
	*功能：显示记录总条数与首页以及1、2、3页码，最多显示当前的三个页码。
	*				实现页面跳转，需要手动实现PaginationChange(this)、PaginationPrev()、PaginationChange(this)、PaginationNext()
	*				这几个函数，分别为：首页、上一页、页面跳转、下一页
	*pagecur:当前页面，第一页为0（注意需要输入数字）
	*pagesize:每一页显示的条数
	*allcount:所有记录总数
	*/
function Pagination(pagecur,pagesize,allcount){
		var allpage = allcount / pagesize;
		var t_page;
		allpage = Math.ceil(allpage); 
		
		var html = '<p style="text-align:right ">--共 有 '+allcount +' 条 记 录 --<br /><br />';
		if((allpage == 1) || (allpage==0)){
			html += '</p>';
		}else if(allpage == 2){
			if(pagecur == 0){
				t_page = pagecur+1;
				html +='<a href="#" class="current" onclick="queryAll('+t_page+')">'+t_page+'</a>';
				html +='<a href="#" onclick="queryAll('+(t_page+1)+')">'+(t_page+1)+'</a>';
				html +='<a href="#" onclick="queryAll('+(t_page+1)+')" class="next">》</a></p>';
			}else{
				t_page = pagecur;
				html +='<a href="#" style="width:40px" onclick="queryAll(1)">首页</a>';
				html +='<a href="#" onclick="queryAll('+(t_page)+')" class="prev">《</a>';
				html +='<a href="#" onclick="queryAll('+t_page+')">'+t_page+'</a>';
				html +='<a href="#" class="current" onclick="queryAll('+(t_page+1)+')">'+(t_page+1)+'</a></p>';
			}
		}else {
			if(pagecur == 0){
				t_page = pagecur+1;
				html +='<a href="#" class="current" onclick="queryAll('+t_page+')">'+t_page+'</a>';
				html +='<a href="#" onclick="queryAll('+(t_page+1)+')">'+(t_page+1)+'</a>';
				html +='<a href="#" onclick="queryAll('+(t_page+2)+')">'+(t_page+2)+'</a>';
				html +='<a href="#" onclick="queryAll('+(t_page+1)+')" class="next">》</a></p>';
			}else if(pagecur == (allpage-1)){
				t_page = pagecur-1;
				html +='<a href="#" style="width:40px" onclick="queryAll(1)">首页</a>';
				html +='<a href="#" onclick="queryAll('+(t_page+1)+')" class="prev">《</a>';
				html +='<a href="#" onclick="queryAll('+t_page+')">'+t_page+'</a>';
				html +='<a href="#" onclick="queryAll('+(t_page+1)+')">'+(t_page+1)+'</a>';
				html +='<a href="#" class="current" onclick="queryAll('+(t_page+2)+')">'+(t_page+2)+'</a></p>';
			}else{
				t_page = pagecur;
				html +='<a href="#" style="width:40px" onclick="queryAll(1)">首页</a>';
				html +='<a href="#" onclick="queryAll('+(t_page)+')" class="prev">《</a>';
				html +='<a href="#" onclick="queryAll('+t_page+')">'+t_page+'</a>';
				html +='<a href="#" class="current" onclick="queryAll('+(t_page+1)+')">'+(t_page+1)+'</a>';
				html +='<a href="#" onclick="queryAll('+(t_page+2)+')">'+(t_page+2)+'</a>';
				html +='<a href="#" onclick="queryAll('+(t_page+2)+')" class="next">》</a></p>';
			}
		}
		return html;
}
