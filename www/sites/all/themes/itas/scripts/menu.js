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
		return false;
		}
		});		
	});
})(jQuery);
