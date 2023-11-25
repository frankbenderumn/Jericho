$(document).ready(function () {
    // hide all dynamic content
    $("[data-role=modal-tab]").each(function () {
        if (!$(this).attr('data-default')) {
            $(this).hide();
        }
    });

    $("[data-role=tab]").each(function () {
        if (!$(this).attr('data-default')) {
            $(this).hide();
        }
    });

    // ################################### MODALS #####################################################

    function exists(self, attr) {
        return $(self).attr(attr);
    }

    $("[data-role=trigger]").click(function () {
        if ($(this).attr('data-type')) {
            let type = $(this).attr('data-type');
            switch (type) {
                case "modal":
                    if (exists($(this), "href")) {
                        let target = $(this).attr("href");
                        console.log("Modal clicked: " + target);
                        $(target).show();
                    }
                    break;
                case "modal-tab":
                    let target;
                    if (exists($(this), "href")) {
                        target = $(this).attr("href");
                    }
                    let parent = $(this).parent().attr("data-cluster");
                    if (parent) {
                        console.warn(parent);
                        $(`[data-cluster=${parent}][data-role=stage] [data-role=modal-tab]`).each(function () {
                            $(this).hide();
                        });
                        $(target).show();
                    } else {
                        console.error("data-cluster for parent not defined for modal-tab");
                    }
                    break;

                case "tab":
                    let target2;
                    if (exists($(this), "href")) {
                        target2 = $(this).attr("href");
                    }
                    let parent2 = $(this).parent().attr("data-cluster");
                    if (parent2) {
                        console.warn(parent2);
                        $(`[data-cluster=${parent2}][data-role=stage] [data-role=tab]`).each(function () {
                            $(this).hide();
                        });
                        $(target2).show();
                    } else {
                        console.error("data-cluster for parent not defined for tab");
                    }
                    break;
                case "page":
                    break;
                default:
                    console.error("Invalid data-type paired with trigger!");
                    break;
            }
        } else {
            console.error("Trigger does not have a data-type");
        }
    });

    // dynamic modal menu (old-code)
    // $("[data-role=trigger]").on('click', function(){
    //     $("[data-role=extend]").each(function(){
    //       $(this).height(0);
    //       $(this).hide();
    //     });
    //     let id = $(this).attr('data-unique');
    //     $("[data-role=show][data-unique='"+id+"']").toggle();
    //     $("[data-role=target][data-unique='"+id+"']").toggle();
    //     $("[data-role=modal][data-unique='"+id+"']").show();
    //     $("[data-role=extend][data-unique='"+id+"']").show().animate({height: '400px'});
    //   });
    //   $("[data-role=select]").click(function(){
    //     let url = $(this).attr('data-selected');
    //     let id = $(this).attr('data-unique');
    //     $("[data-field='"+id+"']").val(url);
    //   });
    if ($(".modal:visible")) {
        $(window).on('click', function (e) {
            if ($(e.target).is(".modal") && !$(e.target).is(".draggable.modal")) {
                $(".modal").each(function () {
                    $(this).hide();
                });
            }
        });
    }
    // ################################### TABS #####################################################

    // ################################### PAGES #####################################################

    // ################################### DRAGGING #####################################################

    // ################################### ALERTS #####################################################

    // ################################### CONSOLE #####################################################

});