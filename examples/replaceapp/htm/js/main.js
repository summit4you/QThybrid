

$(function () {
    'use strict';

   $('#toggle-dirpath').button().click(function (e) {
		e.preventDefault();
		$.get("index.csp", {op:"dir"}, function (data, textStatus){
				$('#path').val(data.path);
				$('#dirpath').val(data.path);
		}, 'json');
    });
	
	 $('#toggle-replace').button().click(function (e) {
		e.preventDefault();
		$.get("index.csp", {op:"replace", before:$('#before').val(), after:$('#after').val(), path:$('#path').val(), filter:$('#filter').val()}, function (data, textStatus){
				bootbox.alert(data.msg, function(){});
				
		}, 'json');

	 });

});