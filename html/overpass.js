
      OpenLayers.Control.Click = OpenLayers.Class(OpenLayers.Control, {
	
        genericUrl: "",
	tolerance: 0.0,
	map: "",
      
	defaultHandlerOptions: {
	  'single': true,
	  'double': false,
	  'pixelTolerance': 0,
	  'stopSingle': false,
	  'stopDouble': false
	},
	
	initialize: function(url, tolerance, map) {
	  this.genericUrl = url;
	  this.tolerance = tolerance;
	  this.map = map;
	
	  this.handlerOptions = OpenLayers.Util.extend(
	    {}, this.defaultHandlerOptions
	  );
	  OpenLayers.Control.prototype.initialize.apply(
	    this, arguments
	  ); 
	  this.handler = new OpenLayers.Handler.Click(
	    this, {
	      'click': this.trigger
	    }, this.handlerOptions
	  );
	}, 
	
	trigger: function(evt) {
	  var lonlat = map.getLonLatFromPixel(evt.xy)
	      .transform(new OpenLayers.Projection("EPSG:900913"),
		         new OpenLayers.Projection("EPSG:4326"));
	      
	  var popup = new OpenLayers.Popup("location_info",
                   new OpenLayers.LonLat(lonlat.lon, lonlat.lat)
		       .transform(new OpenLayers.Projection("EPSG:4326"), new OpenLayers.Projection("EPSG:900913")),
		   null,
                   "Loading ...",
	           true);
	  
	  popup.contentSize = new OpenLayers.Size(400, 400);
	  popup.closeOnMove = true;
	  map.addPopup(popup);

	  var rel_tolerance = this.tolerance * map.getScale();
	  if (rel_tolerance > 0.01)
	    rel_tolerance = 0.01;

	  var request = OpenLayers.Request.GET({
	      url: this.genericUrl + "&bbox="
	          + (lonlat.lon - rel_tolerance) + "," + (lonlat.lat - rel_tolerance) + ","
	          + (lonlat.lon + rel_tolerance) + "," + (lonlat.lat + rel_tolerance),
	      async: false
          });
	  
	  map.removePopup(popup);
	  popup.contentHTML = request.responseText;
	  map.addPopup(popup);
	}
	
      });

// ----------------------------------------------------------------------------
      
      function setStatusText(text)
      {
	  var html_node = document.getElementById("statusline");
	  if (html_node != null)
	  {
            var div = html_node.firstChild;
            div.deleteData(0, div.nodeValue.length);
            div.appendData(text);
	  }
      }

      var zoom_valid = true;
      var load_counter = 0;      
      
      function make_features_added_closure() {
          return function(evt) {
	      setStatusText("");
              if (zoom_valid)
              {
                  --load_counter;
                  if (load_counter <= 0)
                      setStatusText("");
              }
          };
      }

      ZoomLimitedBBOXStrategy = OpenLayers.Class(OpenLayers.Strategy.BBOX, {

          zoom_data_limit: 13,

          initialize: function(zoom_data_limit) {
              this.zoom_data_limit = zoom_data_limit;
          },

          update: function(options) {
              var mapBounds = this.getMapBounds();
              if (this.layer && this.layer.map && this.layer.map.getZoom() < this.zoom_data_limit) {
                  setStatusText("Please zoom in to view data.");
                  zoom_valid = false;

                  this.bounds = null;
              }
              else if (mapBounds !== null && ((options && options.force) ||
                                         this.invalidBounds(mapBounds))) {
                  ++load_counter;
                  setStatusText("Loading data ...");
                  zoom_valid = true;

                  this.calculateBounds(mapBounds);
                  this.resolution = this.layer.map.getResolution();
                  this.triggerRead(options);
              }
          },

          CLASS_NAME: "ZoomLimitedBBOXStrategy"
      });

      function make_large_layer(data_url, color, zoom) {

          var styleMap = new OpenLayers.StyleMap({
              strokeColor: color,
              strokeOpacity: 0.5,
              strokeWidth: 6,
              pointRadius: 10,
              fillColor: color,
              fillOpacity: 0.25
          });
          var layer = new OpenLayers.Layer.Vector(color, {
              strategies: [new ZoomLimitedBBOXStrategy(11)],
              protocol: new OpenLayers.Protocol.HTTP({
                  url: data_url,
                  format: new OpenLayers.Format.OSM()
              }),
              styleMap: styleMap,
              projection: new OpenLayers.Projection("EPSG:4326")
          });

          layer.events.register("featuresadded", layer, make_features_added_closure());

          return layer;
      }

      function make_layer(data_url, color) {
	  return make_large_layer(data_url, color, 13);
      }
      