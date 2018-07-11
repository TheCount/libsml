// Copyright 2011 Juri Glass, Mathias Runge, Nadim El Sayed
// DAI-Labor, TU-Berlin
//
// This file is part of libSML.
//
// libSML is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// libSML is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libSML.  If not, see <http://www.gnu.org/licenses/>.


#include <sml/sml_time.h>
#include <sml/sml_shared.h>
#include <sml/sml_number.h>
#include <stdio.h>

sml_timestamp_local * sml_timestamp_local_init( u32 timestamp, i16 local_offset, i16 season_time_offset ) {
	sml_timestamp_local * t = malloc( sizeof( *t ) );
	if ( t != NULL ) {
		*t = ( sml_timestamp_local ) {
			.timestamp = sml_u32_init( timestamp ),
			.local_offset = sml_i16_init( local_offset ),
			.season_time_offset = sml_i16_init( season_time_offset )
		};
		if ( ( t->timestamp == NULL ) || ( t->local_offset == NULL ) || ( t->season_time_offset == NULL ) ) {
			sml_timestamp_local_free( t );
			t = NULL;
		}
	}

	return t;
}

int sml_timestamp_local_write( sml_timestamp_local * t, sml_buffer * buf ) {
	if ( t == NULL ) {
		sml_buf_optional_write( buf );
		return 0;
	}

	sml_buf_set_type_and_length( buf, SML_TYPE_LIST, 3 );
	sml_u32_write( t->timestamp, buf );
	sml_i16_write( t->local_offset, buf );
	sml_i16_write( t->season_time_offset, buf );

	return 0;
}

sml_timestamp_local * sml_timestamp_local_parse( sml_buffer * buf ) {
	sml_timestamp_local * result = NULL;

	if ( sml_buf_optional_is_skipped( buf ) ) {
		return NULL;
	}

	if ( sml_buf_get_next_type( buf ) != SML_TYPE_LIST ) {
		buf->error = 1;
		goto error;
	}

	if ( sml_buf_get_next_length( buf ) != 3 ) {
		buf->error = 1;
		goto error;
	}

	result = malloc( sizeof( *result ) );
	if ( result == NULL ) {
		buf->error = 1; /* only way to signal errors */
		goto error;
	}
	result->timestamp = NULL;
	result->local_offset = NULL;
	result->season_time_offset = NULL;

	result->timestamp = sml_u32_parse( buf );
	if ( sml_buf_has_errors( buf ) ) {
		goto error;
	}

	result->local_offset = sml_i16_parse( buf );
	if ( sml_buf_has_errors( buf ) ) {
		goto error;
	}

	result->season_time_offset = sml_i16_parse( buf );
	if ( sml_buf_has_errors( buf ) ) {
		goto error;
	}

	return result;

error:
	sml_timestamp_local_free( result );
	return NULL;
}

void sml_timestamp_local_free(sml_timestamp_local *t) {
	if ( t != NULL ) {
		free( t->timestamp );
		free( t->local_offset );
		free( t->season_time_offset );
		free( t );
	}
}

sml_time *sml_time_init() {
	sml_time *t = (sml_time *) malloc(sizeof(sml_time));
	*t = ( sml_time ) {
		.tag = NULL,
		.data.sec_index = NULL
	};
	return t;
}

sml_time *sml_time_parse(sml_buffer *buf) {
	if (sml_buf_optional_is_skipped(buf)) {
		return 0;
	}

	sml_time *tme = sml_time_init();

	if (sml_buf_get_next_type(buf) != SML_TYPE_LIST) {
		buf->error = 1;
		goto error;
	}

	if (sml_buf_get_next_length(buf) != 2) {
		buf->error = 1;
		goto error;
	}

	tme->tag = sml_u8_parse(buf);
	if (sml_buf_has_errors(buf)) goto error;

	switch ( *tme->tag ) {
		case SML_TIME_SEC_INDEX:
			tme->data.sec_index = sml_u32_parse( buf );
			break;

		case SML_TIME_TIMESTAMP:
			tme->data.timestamp = sml_u32_parse( buf );
			break;

		case SML_TIME_TIMESTAMP_LOCAL:
			tme->data.local_timestamp = sml_timestamp_local_parse( buf );
			break;

		default:
			buf->error = 1;
			break;
	}
	if (sml_buf_has_errors(buf)) goto error;

	return tme;

error:
	sml_time_free(tme);
	return 0;
}

void sml_time_write(sml_time *t, sml_buffer *buf) {
	if (t == 0) {
		sml_buf_optional_write(buf);
		return;
	}

	sml_buf_set_type_and_length(buf, SML_TYPE_LIST, 2);
	sml_u8_write(t->tag, buf);
	switch( *t->tag ) {
		case SML_TIME_SEC_INDEX:
			sml_u32_write( t->data.sec_index, buf );
			break;

		case SML_TIME_TIMESTAMP:
			sml_u32_write( t->data.timestamp, buf );
			break;

		case SML_TIME_TIMESTAMP_LOCAL:
			sml_timestamp_local_write( t->data.local_timestamp, buf );
			break;
	}
}

void sml_time_free(sml_time *tme) {
    if (tme) {
	    if ( tme->tag ) {
		switch ( *tme->tag ) {
			case SML_TIME_SEC_INDEX:
				sml_number_free( tme->data.sec_index );
				break;

			case SML_TIME_TIMESTAMP:
				sml_number_free( tme->data.timestamp );
				break;

			case SML_TIME_TIMESTAMP_LOCAL:
				sml_timestamp_local_free( tme->data.local_timestamp );
				break;
		}
		sml_number_free(tme->tag);
	    }
        free(tme);
    }
}

u32 sml_time_get_timestamp(sml_time *time) {
	if (!time->data.timestamp) {
		return 0;
	}
	return *(time->data.timestamp);
}
