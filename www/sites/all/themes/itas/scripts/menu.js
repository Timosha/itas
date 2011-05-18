(function($){
	$(document).ready(function() {
		$('#main-menu ul li').hover(function() {
                        //выполняется при наведении
			$(this).find('ul').fadeIn(300)
		},
		function() {
                        //выполняется когда мы убираем мышку
			$(this).find('ul').hide();
		});
	 $('#main-menu>ul>li>a').click(function() {
       if ($(this).parent().find('ul').length) {
		//return false;
		}
		});
	//Оборачиваем в спаны переключатель языков
	$('.language-switcher-locale-url>li>a').wrap('<span id="lang-sw-center"></span>');		
	$('.language-switcher-locale-url>li').append('<span id="lang-sw-right"></span>');
	$('.language-switcher-locale-url>li').prepend('<span id="lang-sw-left"></span>');
		
	});
})(jQuery);
