
/**
 * @name MFT.ApplinkSliderView
 * 
 * @desc Media Applink Slider View module visual representation
 * 
 * @category    View
 * @filesource  app/view/media/applink/applinksliderView.js
 * @version     2.0
 *
 * @author      Andriy Melnik
 */
 
MFT.ApplinkSliderView = Em.ContainerView.create(MFT.LoadableView, {

    classNames:         ['media_applink_slider_view'],

    elementId:          'media_applink_slider_view',

    childViews:         [
                            'backButton',
                            'initialText',
                            'headerLabel',
                            'footerLabel',
                            'slider'
                        ],

    backButton: MFT.Button.extend({
        classNames:        ['backButton','button'],
        action:            'back',
        target:            'MFT.States',
        icon:              'images/media/ico_back.png',
    }),

    initialText:    MFT.Label.extend({

        elementId:          'initialText',

        classNames:         'initialText',

        contentBinding:     'MFT.ApplinkMediaModel.showInfo.appName'
    }),

    headerLabel:    MFT.Label.extend({

        elementId:          'headerLabel',

        classNames:         'headerLabel',

        contentBinding:     'MFT.ApplinkMediaController.performInteractionInitialText'
    }),

    footerLabel:    MFT.Label.extend({

        elementId:          'footerLabel',

        classNames:         'footerLabel',

        contentBinding:     'MFT.ApplinkMediaController.performInteractionInitialText'
    }),

    slider: Em.ContainerView.create( {//MFT.SelectedIndex, {
          /*
            index:  this.get('index'),

            classNameBindings:  ['isSelected:active'],
            */
            disabled: false,

            classNames:         'control sliderControl',

            elementId:          'sliderControl',

            /** Container components */
            childViews: [
                'minusBtn',
                'led',
                'plusBtn'
            ],

            /** Minus button */
            minusBtn: MFT.Button.extend({
                classNames: 'minus',
                icon:       'images/common/minus-ico.png',
                //action:     'onDecrease',
                //target:     'MFT.MCSController',

                //timer:      FLAGS.MCS_EMULATE_CAN ? 200 : 0,
                /*
                actionUp:   function() {
                    if(!this._parentView._parentView.get('disabled')) {
                        this._super();
                        
                        if (!FLAGS.MCS_EMULATE_CAN) {
                            MFT.MCSController.model.send(this._parentView.index,'off');
                        }
                        MFT.MCSController.set('highlighted', false);
                    }
                },

                actionDown: function() {
                    if(!this._parentView._parentView.get('disabled')) {
                        MFT.MCSController.set('highlighted', true);
                        this._super();
                    }
                },
                
                mouseLeave: function() {
                    if(!this._parentView._parentView.get('disabled')) {
                        if (!FLAGS.MCS_EMULATE_CAN && this.pressed) {
                            MFT.MCSController.model.send(this._parentView.index,'off');
                        }
                        MFT.MCSController.set('highlighted', false);
                        
                        this._super();
                    }
                }*/
            }),

            /** adjust */
            led: MFT.Indicator.create({
                classNames:         'ledContainer ico',
                //action:             'select',
                //target:             'MFT.MCSController',
                //contentBinding:     'MFT.MCSController.activeSeat.' + (this.get('index') - 1),
                contentBinding:     'MFT.ApplinkMediaModel.applinkSliderContent',
                indActiveClass:     'led',
                indDefaultClass:    'led-inactive',
/*
                actionDown: function() {
                    if(!this._parentView._parentView.get('disabled')) {
                        MFT.MCSController.set('highlighted', true);
                        this._super();
                    }
                },

                actionUp:   function() {
                    if(!this._parentView._parentView.get('disabled')) {
                        MFT.MCSController.set('highlighted', false);
                    }
                },
                
                mouseLeave: function() {
                    if(!this._parentView._parentView.get('disabled')) {
                        MFT.MCSController.set('highlighted', false);
                    }
                }*/
            }),

            /** Plus button */
            plusBtn: MFT.Button.extend({
                classNames: 'plus',
                icon:       'images/common/plus-ico.png',
                //action:     'onIncrease',
                //target:     'MFT.MCSController',
                
                timer:      FLAGS.MCS_EMULATE_CAN ? 200 : 0,
/*
                actionUp:   function() {
                    if(!this._parentView._parentView.get('disabled')) {
                        this._super();
                        
                        if (!FLAGS.MCS_EMULATE_CAN) {
                            MFT.MCSController.model.send(this._parentView.index,'off');
                        }
                        MFT.MCSController.set('highlighted', false);
                    }
                },

                actionDown: function() {
                    if(!this._parentView._parentView.get('disabled')) {
                        MFT.MCSController.set('highlighted', true);
                        this._super();
                    }
                },
                
                mouseLeave: function() {
                    if(!this._parentView._parentView.get('disabled')) {
                        if (!FLAGS.MCS_EMULATE_CAN && this.pressed) {
                            MFT.MCSController.model.send(this._parentView.index,'off');
                        }
                        MFT.MCSController.set('highlighted', false);
                        
                        this._super();
                    }
                }*/
            })
        })

});