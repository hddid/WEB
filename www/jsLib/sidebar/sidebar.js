/* zw
 * www.bctech.org.cn
 */
jQuery(function ($) {
	var currentMenu = null;
	$('#sidebar>ul>li').each(function () {
		if ($(this).find('li').length == 0) {
			$(this).addClass('nosubmenu');
		}
	})
	$('#sidebar>ul>li[class!="nosubmenu"]>a').each(function () {
		if (!$(this).parent().hasClass('current')) {
			$(this).parent().find('ul:first').hide();
		} else {
			currentMenu = $(this);
		}
		$(this).click(function () {
			$('#sidebar>ul>li.current').removeClass('current');
			if (currentMenu != null && currentMenu.text() != $(this).text()) {
				currentMenu.parent().find('ul:first').slideUp();
			}
			if (currentMenu != null && currentMenu.text() == $(this).text()) {
				currentMenu.parent().find('ul:first').slideUp();
				currentMenu = null;
			} else {
				currentMenu = $(this);
				currentMenu.parent().addClass('current');
				currentMenu.parent().find('ul:first').slideDown();
			}
			return false;
		});
	});
});