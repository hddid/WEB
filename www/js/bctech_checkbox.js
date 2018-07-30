/* zw
 * www.bctech.org.cn
 */
(function($){
	var methods = {
		init : function (options){
			return this.each(function (){
				var $this = $(this);
				
				//防止单选按钮有多个选中
				if($this.attr("type") == 'radio' && $this.hasClass("checked") && $this.attr('name') != undefined && $this.attr('name') != ''){
					$("[type=radio][name=" + $this.attr('name') + "].checked").removeClass('checked');
					$this.addClass('checked');
				}
				
				$this.click(function (){
					var $this = $(this);
					var type = $this.attr("type");
					
					if(type == 'radio' && $this.hasClass("checked")) return;
					
					var bctech_checkbox_cb = $this.attr('cb');
					if(bctech_checkbox_cb && window[bctech_checkbox_cb]){
						window[bctech_checkbox_cb].apply(this, [changeChecked, this]);
					}else{
						changeChecked(this);
					}
				});
			});
		},
		getValue : function (){
			return $(this).hasClass("checked");
		},
		setValue : function (v){
			return this.each(function (){
				var $this = $(this);
				var type = $this.attr("type");
				if(type == 'radio'){
					if($(this).hasClass("checked")) return false;
				}else{
					if($(this).hasClass("checked") == (v == true || v == 'checked')) return false;
				}
				
				changeChecked(this);
			});
		}
	}
	
	var changeChecked = function(o){
		var $this = $(o);
		var type = $this.attr("type");
		
		if(type == 'radio'){ // radio 只能选中，不能取消选中
			if('' == $this.attr('name')){
				$this.addClass('checked');
			}else{
				$("[type=radio][name=" + $this.attr('name') + "].checked").removeClass('checked');
				$this.addClass('checked');
			}
		}else{
			$this.toggleClass('checked');
		}
	}
	
	$.fn.bctechCheckbox = function(method) {
		if ( methods[method] ) {
			return methods[method].apply( this, Array.prototype.slice.call( arguments, 1 ));
		} else if ( typeof method === 'object' || ! method ) {
			return methods.init.apply(this, arguments );  
		}else{
			return this;
		}
	};
	
})(jQuery);

$(function (){
	$(".bctech_checkbox").bctechCheckbox();
});