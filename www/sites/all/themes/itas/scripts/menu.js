(function($){
	$(document).ready(function() {
		$('#main-menu ul li').hover(function() {
                        //����������� ��� ���������
			$(this).find('ul').fadeIn(300)
		},
		function() {
                        //����������� ����� �� ������� �����
			$(this).find('ul').hide();
		});
	 $('#main-menu>ul>li>a').click(function() {
       if ($(this).parent().find('ul').length) {
		return false;
		}
		});		
	});
})(jQuery);
