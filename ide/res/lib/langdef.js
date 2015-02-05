function LanguageDefinition(){
	this.m_existing_tokens={};
	this.m_big_chars=[];
	this.m_bracket_types=[];
	this.m_entry_states=[];
	this.m_coloring_rules=[];
}
var REAL_TYPE_MOV=0;
var REAL_TYPE_XOR=1;
var REAL_TYPE_ADD=2;
LanguageDefinition.prototype={
	DefineToken:function(stoken){
		var n=this.m_existing_tokens[stoken];
		if(n!=undefined){return n;}
		n=(this.m_big_chars.length);
		this.m_big_chars.push(stoken)
		if(n>=128){throw new Error("too many bigchars, 127 should have been enough")}
		if(Duktape.__byte_length(stoken)>64){throw new Error("bigchar too long, 64 should have been enough")}
		this.m_existing_tokens[stoken]=n
		return n;
	},
	DefineDelimiter:function(type,stok0,stok1){
		var real_type;
		var tok0=this.DefineToken(stok0);
		var tok1=this.DefineToken(stok1);
		if(type=="nested"){
			real_type=REAL_TYPE_ADD;
		}else{
			if(tok0==tok1){
				real_type=REAL_TYPE_XOR;
			}else{
				real_type=REAL_TYPE_MOV;
			}
			if(type!="key"&&type=="normal"){
				throw new Error("invalid delimiter type '@1'".replace("@1",type));
			}
		}
		var bid=this.m_bracket_types.length;
		this.m_bracket_types.push({type:real_type,is_key:(type=="key"),bid:bid,tok0:tok0,tok1:tok1});
		return bid;
	},
	AddColorRule:function(bid,color_name){
		//coloring... bracket range + delta, later-overwrite-earlier rules
		//nested brackets not allowed
		if(!(bid<this.m_bracket_types.length&&bid>=0)){
			throw new Error("bad delimiter id");
		}
		!?
		this.m_coloring_rules.push({bid:bid,color_name:color_name});
	},
	/////////////////
	isInside:function(bid){
		return m_inside_mask&(1<<bid);
	},
	Enable:function(bid){
		m_enabling_mask|=(1<<bid);
	},
	Disable:function(bid){
		m_enabling_mask&=~(1<<bid);
	},
	/////////////////
	Finalize:function(fenabler){
		var bras=this.m_bracket_types;
		bras.sort(function(a,b){
			return (a.is_key-b.is_key||a.type-b.type);
		})
		var n_keys=bras.length;
		for(var i=0;i<bras.length;i++){
			if(!bras[i].is_key){n_keys=i;break;}
		}
		if(n_keys>12){
			throw new Error("too many key brackets, do you really have that much inter-bracket dependency?")
		}
		if(bras.length>=32){
			throw new Error("too many bracket types, 31 types is surely enough?")
		}
		var n_combos=(1<<n_keys);
		var bidmap={};
		for(var j=0;j<bras.length;j++){
			bidmap[bras[j].bid]=j;
		}
		for(var mask_i=0;mask_i<n_combos;i++){
			this.m_enabling_mask=0;
			this.m_inside_mask=0;
			for(var j=0;j<n_keys;j++){
				if(mask_i&(1<<j)){
					this.m_inside_mask|=(1<<bras[i].bid);
				}
			}
			this.m_enabling_mask|=this.m_inside_mask;
			fenabler(this);
			if((this.m_enabling_mask&this.m_inside_mask)!=this.m_inside_mask){
				//self-contradicting, ignore it
				continue
			}
			var raw_enabling_mask=0;
			for(var j=0;j<bras.length;j++){
				if(this.m_enabling_mask&(1<<bras[j].bid)){
					raw_enabling_mask|=(1<<j);
				}
			}
			this.m_entry_states.push({inside:mask_i,enabled:raw_enabling_mask})
		}
		if(this.m_entry_states.length>64){
			throw new Error("there are @1 entry states, but the system only supports 64".replace("@1",this.m_entry_states.length))
		}
		//translate m_coloring_rules
		if(this.m_coloring_rules.length>63){
			throw new Error("there are @1 coloring rules, but the system only supports 63".replace("@1",this.m_coloring_rules.length))
		}
		for(var i=0;i<this.m_coloring_rules.length;i++){
			this.m_coloring_rules[i].bid=bidmap[this.m_coloring_rules[i].bid];
		}
	},
};
