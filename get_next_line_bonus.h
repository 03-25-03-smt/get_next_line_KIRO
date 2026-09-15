/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_bonus.h                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vladyslb <vladyslb@student.42prague.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/15 09:00:00 by vladyslb          #+#    #+#             */
/*   Updated: 2026/09/15 09:00:00 by vladyslb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GET_NEXT_LINE_BONUS_H
# define GET_NEXT_LINE_BONUS_H

# include <unistd.h>
# include <stdlib.h>
# include <sys/types.h>
# include <limits.h>

# ifndef BUFFER_SIZE
#  define BUFFER_SIZE 42
# endif

# define MAX_FILES 1024

char	*get_next_line(int fd);

size_t	strlen_at(const char *s, int end);
char	*find_chr(const char *s, int c);
void	*ft_memcpy(void *dest, const void *src, size_t n);
char	*cpy_buffer(const char *buffer, size_t start, size_t len);
char	*merge_previous_and_current(char *previous, const char *current);

#endif
